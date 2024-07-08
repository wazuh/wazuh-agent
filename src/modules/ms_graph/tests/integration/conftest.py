# Copyright (C) 2015-2024, Wazuh Inc.
# Created by Wazuh, Inc. <info@wazuh.com>.
# This program is free software; you can redistribute it and/or modify it under the terms of GPLv2

import pytest
import subprocess
import os
import sys
from pathlib import Path
from typing import List

from wazuh_testing import session_parameters
from wazuh_testing.constants import platforms
from wazuh_testing.constants.paths import ROOT_PREFIX
from wazuh_testing.constants.paths.logs import WAZUH_LOG_PATH
from wazuh_testing.constants.paths import WAZUH_PATH
from wazuh_testing.logger import logger
from wazuh_testing.modules.modulesd import patterns
from wazuh_testing.tools.monitors.file_monitor import FileMonitor
from wazuh_testing.utils import callbacks, configuration, services
from wazuh_testing.utils.file import remove_file, truncate_file


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


@pytest.fixture()
def daemons_handler(request: pytest.FixtureRequest) -> None:
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
            services.control_service('restart')
        else:
            for daemon in daemons:
                logger.debug(f"Restarting {daemon}")
                # Restart daemon instead of starting due to legacy used fixture in the test suite.
                services.control_service('restart', daemon=daemon)

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
        services.control_service('stop')
    else:
        for daemon in daemons:
            logger.debug(f"Stopping {daemon}")
            services.control_service('stop', daemon=daemon)


@pytest.fixture()
def wait_for_msgraph_start():
    # Wait for module ms-graph starts
    wazuh_log_monitor = FileMonitor(WAZUH_LOG_PATH)
    wazuh_log_monitor.start(callback=callbacks.generate_callback(patterns.MODULESD_STARTED, {
                              'integration': 'ms-graph'
                          }))
    assert (wazuh_log_monitor.callback_result == None), f'Error invalid configuration event not detected'


@pytest.fixture(scope="session")
def proxy_setup():
    RESPONSES_PATH = Path(Path(__file__).parent, 'test_API', 'data', 'response_samples', 'responses.json')
    m365proxy = subprocess.Popen(["/tmp/m365proxy/m365proxy", "--mocks-file", RESPONSES_PATH])
    # Configurate proxy for Wazuh (will only work for systemctl start/restart)
    subprocess.run("systemctl set-environment http_proxy=http://localhost:8000", shell=True)
    remove_file(os.path.join(WAZUH_PATH, 'var', 'wodles', 'ms-graph-tenant_id-resource_name-resource_relationship'))

    yield

    subprocess.run("systemctl unset-environment http_proxy", shell=True)
    m365proxy.kill()
    m365proxy.wait()
