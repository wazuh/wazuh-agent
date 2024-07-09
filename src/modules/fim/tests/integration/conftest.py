"""
Copyright (C) 2015-2024, Wazuh Inc.
Created by Wazuh, Inc. <info@wazuh.com>.
This program is free software; you can redistribute it and/or modify it under the terms of GPLv2
"""
import os
from time import sleep
import distro
import pytest
import re
import subprocess
import sys
from typing import List

if sys.platform == 'win32':
    import win32con

from typing import Any
from pathlib import Path

from wazuh_testing import session_parameters
from wazuh_testing.constants.paths import ROOT_PREFIX
from wazuh_testing.constants.paths.logs import WAZUH_LOG_PATH
from wazuh_testing.constants.platforms import LINUX, WINDOWS, MACOS, CENTOS, UBUNTU, DEBIAN
from wazuh_testing.logger import logger
from wazuh_testing.modules.fim.patterns import MONITORING_PATH, EVENT_TYPE_SCAN_END
from wazuh_testing.modules.fim.utils import create_registry, delete_registry
from wazuh_testing.tools.monitors.file_monitor import FileMonitor
from wazuh_testing.tools.simulators.authd_simulator import AuthdSimulator
from wazuh_testing.tools.simulators.remoted_simulator import RemotedSimulator
from wazuh_testing.utils import configuration, file
from wazuh_testing.utils.callbacks import generate_callback
from wazuh_testing.utils.services import control_service


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
    _platforms = set([LINUX,
                      WINDOWS,
                      MACOS])

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


# - - - - - - - - - - - - - - - - - - - - - - -End of Pytest configuration - - - - - - - - - - - - - - - - -


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
            file.truncate_file(log_file)

    yield

    for log_file in log_files:
        if os.path.isfile(os.path.join(ROOT_PREFIX, log_file)):
            file.truncate_file(log_file)


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
def file_to_monitor(test_metadata: dict) -> Any:
    path = test_metadata.get('file_to_monitor')
    path = os.path.abspath(path)
    data = test_metadata.setdefault('content', '')
    isBinary = test_metadata.setdefault('binary_content', False)

    if isBinary:
        data = data.encode('utf-8')

    file.write_file(path, data)

    yield path

    file.remove_file(path)


@pytest.fixture()
def folder_to_monitor(test_metadata: dict) -> None:
    path = test_metadata.get('folder_to_monitor')
    path = os.path.abspath(path)

    file.recursive_directory_creation(path)

    yield path

    file.delete_path_recursively(path)


@pytest.fixture()
def fill_folder_to_monitor(test_metadata: dict) -> None:
    path = test_metadata.get('folder_to_monitor')
    amount = test_metadata.get('files_amount')
    amount = 2 if not amount else amount
    max_retries = 3
    retry_delay = 1

    if not file.exists(path):
        file.recursive_directory_creation(path)

    [file.write_file(Path(path, f'test{i}.log'), 'content') for i in range(amount)]

    yield

    for i in range(amount):
        retry_count = 0
        while retry_count < max_retries:
            try:
                file.remove_file(Path(path, f'test{i}.log'))
                break
            except Exception as e:
                print(f"Error deleting file {i}: {e}")
                retry_count += 1
                if retry_count == max_retries:
                    print(f"Failed to delete file {i} after {max_retries} attempts.")
                    break
                else:
                    print(f"Retrying in {retry_delay} seconds...")
                    sleep(retry_delay)

@pytest.fixture()
def start_monitoring() -> None:
    FileMonitor(WAZUH_LOG_PATH).start(generate_callback(MONITORING_PATH))


@pytest.fixture(scope='module', autouse=True)
def set_agent_config(request: pytest.FixtureRequest):
    if not hasattr(request.module, 'test_configuration'):
        return

    configurations = getattr(request.module, 'test_configuration')
    agent_conf = {"section": "client", "elements": [
        {"server": {"elements": [
            {"address": {"value": "127.0.0.1"}},
            {"port": {"value": 1514}},
            {"protocol": {"value": "tcp"}}]}}]}

    for index, _ in enumerate(configurations):
        configurations[index]['sections'].append(agent_conf)

    request.module.test_configuration = configurations


@pytest.fixture(scope='session', autouse=True)
def install_audit():
    """Automatically install auditd before test session on linux distros."""
    if sys.platform == WINDOWS or sys.platform == MACOS:
        return

    # Check distro
    linux_distro = distro.id()

    if re.match(linux_distro, CENTOS):
        package_management = "yum"
        audit = "audit"
        option = "--assumeyes"
    elif re.match(linux_distro, UBUNTU) or re.match(linux_distro, DEBIAN):
        package_management = "apt-get"
        audit = "auditd"
        option = "--yes"
    else:
        raise ValueError(
            f"Linux distro ({linux_distro}) not supported for install audit")

    subprocess.run([package_management, "install", audit, option], check=True)
    subprocess.run(["service", "auditd", "start"], check=True)


@pytest.fixture()
def create_links_to_file(folder_to_monitor: str, file_to_monitor: str, test_metadata: dict) -> None:
    hardlink_amount = test_metadata.get('hardlink_amount', 0)
    symlink_amount = test_metadata.get('symlink_amount', 0)

    def hardlink(i: int):
        Path(folder_to_monitor, f'test_h{i}').symlink_to(file_to_monitor)

    def symlink(i: int):
        Path(folder_to_monitor, f'test_s{i}').hardlink_to(file_to_monitor)

    [hardlink(i) for i in range(hardlink_amount)]
    [symlink(i) for i in range(symlink_amount)]

    yield

    [file.remove_file(f'test_h{i}') for i in range(hardlink_amount)]
    [file.remove_file(f'test_s{i}') for i in range(symlink_amount)]


@pytest.fixture()
def create_registry_key(test_metadata: dict) -> None:
    key = win32con.HKEY_LOCAL_MACHINE
    sub_key = test_metadata.get('sub_key')
    arch = win32con.KEY_WOW64_64KEY if test_metadata.get('arch') == 'x64' else win32con.KEY_WOW64_32KEY

    create_registry(key, sub_key, arch)

    yield

    delete_registry(key, sub_key, arch)


@pytest.fixture()
def detect_end_scan(test_metadata: dict) -> None:
    wazuh_log_monitor = FileMonitor(WAZUH_LOG_PATH)
    wazuh_log_monitor.start(timeout=60, callback=generate_callback(EVENT_TYPE_SCAN_END))
    assert wazuh_log_monitor.callback_result


@pytest.fixture()
def create_paths_files(test_metadata: dict) -> str:
    to_edit = test_metadata.get('path_or_files_to_create')

    if not isinstance(to_edit, list):
        raise TypeError(f"`files` should be a 'list', not a '{type(to_edit)}'")

    created_files = []
    for item in to_edit:
        item_path = Path(item)
        if item_path.exists():
            raise FileExistsError(f"`{item_path}` already exists.")

        # If file does not have suffixes, consider it a directory
        if item_path.suffixes == []:
            # Add a dummy file to the target directory to create the directory
            created_files.extend(file.create_parent_directories(
                Path(item_path).joinpath('dummy.file')))
        else:
            created_files.extend(file.create_parent_directories(item_path))

            file.write_file(file_path=item_path, data='')
            created_files.append(item_path)

    yield to_edit

    for item in to_edit:
        item_path = Path(item)
        file.delete_path_recursively(item_path)


@pytest.fixture(autouse=True)
def autostart_simulators(request: pytest.FixtureRequest) -> None:
    """
    Fixture for starting simulators in wazuh-agent executions.

    This fixture starts both Authd and Remoted simulators, and when the test function is not already
    using the simulator fixture, if it does use one of them, only start the remaining simulator.

    This is required so all wazuh-agent instances are being tested with the wazuh-manager connection
    being mocked.
    """
    create_authd = 'authd_simulator' not in request.fixturenames
    create_remoted = 'remoted_simulator' not in request.fixturenames

    authd = AuthdSimulator() if create_authd else None
    remoted = RemotedSimulator() if create_remoted else None

    authd.start() if create_authd else None
    remoted.start() if create_remoted else None

    yield

    authd.shutdown() if create_authd else None
    remoted.shutdown() if create_remoted else None
