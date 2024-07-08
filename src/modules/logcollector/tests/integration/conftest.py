'''
copyright: Copyright (C) 2015-2024, Wazuh Inc.
           Created by Wazuh, Inc. <info@wazuh.com>.
           This program is free software; you can redistribute it and/or modify it under the terms of GPLv2
'''
import os
import sys
import subprocess
import pytest
from typing import List

from os.path import join as path_join

from wazuh_testing import session_parameters
from wazuh_testing.constants import platforms
from wazuh_testing.constants.paths import ROOT_PREFIX
from wazuh_testing.constants.paths.logs import WAZUH_LOG_PATH
from wazuh_testing.constants.paths import WAZUH_PATH
from wazuh_testing.constants.paths.configurations import WAZUH_CONF_PATH
from wazuh_testing.constants.daemons import LOGCOLLECTOR_DAEMON
from wazuh_testing.logger import logger
from wazuh_testing.modules.logcollector.patterns import LOGCOLLECTOR_MODULE_START
from wazuh_testing.tools.monitors.file_monitor import FileMonitor
from wazuh_testing.utils import callbacks, configuration
from wazuh_testing.utils.services import control_service
from wazuh_testing.utils.file import truncate_file, replace_regex_in_file, write_json_file

# Logcollector internal paths
LOGCOLLECTOR_OFE_PATH = path_join(WAZUH_PATH, 'queue', 'logcollector', 'file_status.json')


# - - - - - - - - - - - - - - - - - - - - - - - - -Pytest configuration - - - - - - - - - - - - - - - - - - - - - - -


def pytest_addoption(parser: pytest.Parser) -> None:
    """Add command-line options to the tests.

    Args:
        parser (pytest.Parser): Parser for command line arguments and ini-file values.
    """
    parser.addoption(
        "--tier",
        action="append",
        metavar="level",
        default=None,
        type=int,
        help="only run tests with a tier level equal to 'level'",
    )
    parser.addoption(
        "--tier-minimum",
        action="store",
        metavar="minimum_level",
        default=-1,
        type=int,
        help="only run tests with a tier level greater or equal than 'minimum_level'"
    )
    parser.addoption(
        "--tier-maximum",
        action="store",
        metavar="maximum_level",
        default=sys.maxsize,
        type=int,
        help="only run tests with a tier level less or equal than 'minimum_level'"
    )


def pytest_collection_modifyitems(config: pytest.Config, items: List[pytest.Item]) -> None:
    """Deselect tests that do not match with the specified environment or tier.

    Args:
        config (pytest.Config): Access to configuration values, pluginmanager and plugin hooks.
        items (list): List of items where each item is a basic test invocation.
    """
    selected_tests = []
    deselected_tests = []
    _platforms = set([platforms.LINUX,
                      platforms.WINDOWS,
                      platforms.MACOS])

    for item in items:
        supported_platforms = _platforms.intersection(
            mark.name for mark in item.iter_markers())
        plat = sys.platform

        selected = True
        if supported_platforms and plat not in supported_platforms:
            selected = False

        # Consider only first mark
        levels = [mark.kwargs['level']
                  for mark in item.iter_markers(name="tier")]
        if levels and len(levels) > 0:
            tiers = item.config.getoption("--tier")
            if tiers is not None and levels[0] not in tiers:
                selected = False
            elif item.config.getoption("--tier-minimum") > levels[0]:
                selected = False
            elif item.config.getoption("--tier-maximum") < levels[0]:
                selected = False
        if selected:
            selected_tests.append(item)
        else:
            deselected_tests.append(item)

    config.hook.pytest_deselected(items=deselected_tests)
    items[:] = selected_tests


# - - - - - - - - - - - - - - - - - - - - - - -End of Pytest configuration - - - - - - - - - - - - - - - - - - - - - - -


@pytest.fixture()
def set_wazuh_configuration(test_configuration: dict) -> None:
    """Set wazuh configuration

    Args:
        test_configuration (dict): Configuration template data to write in the ossec.conf
    """
    # Save current configuration
    backup_config = configuration.get_wazuh_conf()

    # Configuration for testing
    test_config = configuration.set_section_wazuh_conf(test_configuration.get('sections'))

    # Set new configuration
    configuration.write_wazuh_conf(test_config)

    # Set current configuration
    session_parameters.current_configuration = test_config

    yield

    # Restore previous configuration
    configuration.write_wazuh_conf(backup_config)


@pytest.fixture()
def truncate_monitored_files() -> None:
    """Truncate all the log files and json alerts files before and after the test execution"""
    log_files = [WAZUH_LOG_PATH]

    for log_file in log_files:
        if os.path.isfile(os.path.join(ROOT_PREFIX, log_file)):
            truncate_file(log_file)

    yield

    for log_file in log_files:
        if os.path.isfile(os.path.join(ROOT_PREFIX, log_file)):
            truncate_file(log_file)


@pytest.fixture()
def configure_local_internal_options(request: pytest.FixtureRequest, test_metadata) -> None:
    """Configure the local internal options file.

    Takes the `local_internal_options` variable from the request.
    The `local_internal_options` is a dict with keys and values as the Wazuh `local_internal_options` format.
    E.g.: local_internal_options = {'monitord.rotate_log': '0', 'syscheck.debug': '0' }

    Args:
        request (pytest.FixtureRequest): Provide information about the current test function which made the request.
        test_metadata (map): Data with configuration parameters
    """
    try:
        local_internal_options = request.param
    except AttributeError:
        try:
            local_internal_options = getattr(request.module, 'local_internal_options')
        except AttributeError:
            raise AttributeError('Error when using the fixture "configure_local_internal_options", no '
                                 'parameter has been passed explicitly, nor is the variable local_internal_options '
                                 'found in the module.') from AttributeError

    backup_local_internal_options = configuration.get_local_internal_options_dict()

    if test_metadata and 'local_internal_options' in test_metadata:
        for key in test_metadata['local_internal_options']:
            local_internal_options[key] = test_metadata['local_internal_options'][key]

    configuration.set_local_internal_options_dict(local_internal_options)

    yield

    configuration.set_local_internal_options_dict(backup_local_internal_options)


def daemons_handler_implementation(request: pytest.FixtureRequest) -> None:
    """Helper function to handle Wazuh daemons.

    It uses `daemons_handler_configuration` of each module in order to configure the behavior of the fixture.

    The  `daemons_handler_configuration` should be a dictionary with the following keys:
        daemons (list, optional): List with every daemon to be used by the module. In case of empty a ValueError
            will be raised
        all_daemons (boolean): Configure to restart all wazuh services. Default `False`.
        ignore_errors (boolean): Configure if errors in daemon handling should be ignored. This option is available
        in order to use this fixture along with invalid configuration. Default `False`

    Args:
        request (pytest.FixtureRequest): Provide information about the current test function which made the request.
    """
    daemons = []
    ignore_errors = False
    all_daemons = False

    if config := getattr(request.module, 'daemons_handler_configuration', None):
        if 'daemons' in config:
            daemons = config['daemons']
            if not daemons or len(daemons) == 0 or type(daemons) not in [list, tuple]:
                logger.error('Daemons list/tuple is not set')
                raise ValueError

        if 'all_daemons' in config:
            logger.debug(f"Wazuh control set to {config['all_daemons']}")
            all_daemons = config['all_daemons']

        if 'ignore_errors' in config:
            logger.debug(f"Ignore error set to {config['ignore_errors']}")
            ignore_errors = config['ignore_errors']
    else:
        logger.debug("Wazuh control set to 'all_daemons'")
        all_daemons = True

    try:
        if all_daemons:
            logger.debug('Restarting wazuh using wazuh-control')
            control_service('restart')
        else:
            for daemon in daemons:
                logger.debug(f"Restarting {daemon}")
                # Restart daemon instead of starting due to legacy used fixture in the test suite.
                control_service('restart', daemon=daemon)

    except ValueError as value_error:
        logger.error(f"{str(value_error)}")
        if not ignore_errors:
            raise value_error
    except subprocess.CalledProcessError as called_process_error:
        logger.error(f"{str(called_process_error)}")
        if not ignore_errors:
            raise called_process_error

    yield

    if all_daemons:
        logger.debug('Stopping wazuh using wazuh-control')
        control_service('stop')
    else:
        for daemon in daemons:
            logger.debug(f"Stopping {daemon}")
            control_service('stop', daemon=daemon)


@pytest.fixture()
def daemons_handler(request: pytest.FixtureRequest) -> None:
    """Wrapper of `daemons_handler_implementation` which contains the general implementation.

    Args:
        request (pytest.FixtureRequest): Provide information about the current test function which made the request.
    """
    yield from daemons_handler_implementation(request)


@pytest.fixture(scope='module')
def daemons_handler_module(request: pytest.FixtureRequest) -> None:
    """Wrapper of `daemons_handler_implementation` which contains the general implementation.

    Args:
        request (pytest.FixtureRequest): Provide information about the current test function which made the request.
    """
    yield from daemons_handler_implementation(request)


@pytest.fixture()
def stop_logcollector(request):
    """Stop wazuh-logcollector and truncate logs file."""
    control_service('stop', daemon=LOGCOLLECTOR_DAEMON)
    truncate_file(WAZUH_LOG_PATH)


@pytest.fixture()
def wait_for_logcollector_start(request):
    # Wait for logcollector thread to start
    log_monitor = FileMonitor(WAZUH_LOG_PATH)
    log_monitor.start(callback=callbacks.generate_callback(LOGCOLLECTOR_MODULE_START))
    assert (log_monitor.callback_result != None), f'Error logcollector start event not detected'


@pytest.fixture()
def remove_all_localfiles_wazuh_config(request):
    """Configure a custom settting for testing. Restart Wazuh is needed for applying the configuration. """
    # Backup the original configuration
    backup_config = configuration.get_wazuh_conf()

    # Remove localfiles from the configuration
    list_tags = [r"<localfile>[\s\S]*?<\/localfile>"]
    replace_regex_in_file(list_tags, [''] * len(list_tags), WAZUH_CONF_PATH, True)

    yield
    configuration.write_wazuh_conf(backup_config)


@pytest.fixture()
def reset_ofe_status(request: pytest.FixtureRequest, test_metadata: dict):
    """Reset the status file of the logcollector only future events."""

    def get_journal_last_log_timestamp():
        '''
        Get the timestamp of the last log message in the journal.

        Returns:
            int: The timestamp of the last log message in the journal.
        '''
        from subprocess import Popen, PIPE
        from shlex import split

        # Get the last log message in the journal
        command = 'journalctl -o json -n1'
        process = Popen(split(command), stdout=PIPE, stderr=PIPE)
        output, error = process.communicate()

        if error:
            raise Exception(f"Error getting the last log message from the journal: {error.decode()}")

        # Get the timestamp of the last log message
        import json
        log_message = json.loads(output.decode())
        return log_message.get('_SOURCE_REALTIME_TIMESTAMP')

    def get_ofe_journald():
        '''
        Get the status of the logcollector for journald.

        Set the timestamp of the last log message in the journal as the timestamp for the journald.
        if the test_metadata contains the key 'force_timestamp', the value of this key will be used as the timestamp.

        Returns:
            dict: The status of the logcollector for journald.
        '''

        if 'force_timestamp' in test_metadata:
            epoch_timestamp = test_metadata['force_timestamp']
        else:
            epoch_timestamp = get_journal_last_log_timestamp()

        status: dict = { "timestamp": str(epoch_timestamp) }
        return status

    # File status for logcollector
    file_status: dict = {}

    # Configure the file status for each logreader
    file_status['journald'] = get_ofe_journald()

    # Write the file status
    write_json_file(LOGCOLLECTOR_OFE_PATH, file_status)


@pytest.fixture()
def pre_send_journal_logs(request: pytest.FixtureRequest, test_metadata: dict):
    """Send log messages to the journal before starting the logcollector."""
    from utils import send_log_to_journal

    if 'pre_input_logs' not in test_metadata:
        raise Exception(f"The test_metadata does not contain the key 'pre_input_logs'")
    else:
        for log_message in test_metadata['pre_input_logs']:
            send_log_to_journal(log_message)
