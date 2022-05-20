/*
 * Copyright (C) 2015, Wazuh Inc.
 * May 03, 2022
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include <sys/time.h>

typedef struct _agent_syscheck_t {
    uint64_t syscheck_queries;
    struct timeval syscheck_time;
    uint64_t fim_file_queries;
    uint64_t fim_file_time;
    uint64_t fim_registry_queries;
    uint64_t fim_registry_time;
} agent_syscheck_t;

typedef struct _agent_syscollector_t {
    uint64_t syscollector_processes_queries;
    uint64_t syscollector_processes_time;
    uint64_t syscollector_packages_queries;
    uint64_t syscollector_packages_time;
    uint64_t syscollector_hotfixes_queries;
    uint64_t syscollector_hotfixes_time;
    uint64_t syscollector_ports_queries;
    uint64_t syscollector_ports_time;
    uint64_t syscollector_network_protocol_queries;
    uint64_t syscollector_network_protocol_time;
    uint64_t syscollector_network_address_queries;
    uint64_t syscollector_network_address_time;
    uint64_t syscollector_network_iface_queries;
    uint64_t syscollector_network_iface_time;
    uint64_t syscollector_hwinfo_queries;
    uint64_t syscollector_hwinfo_time;
    uint64_t syscollector_osinfo_queries;
    uint64_t syscollector_osinfo_time; ///nuevas 
    uint64_t process_queries; //deprecated
    uint64_t process_time;
    uint64_t package_queries;
    uint64_t package_time;
    uint64_t hotfix_queries;
    uint64_t hotfix_time;
    uint64_t port_queries;
    uint64_t port_time;
    uint64_t netproto_queries;
    uint64_t netproto_time;
    uint64_t netaddr_queries;
    uint64_t netaddr_time;
    uint64_t netinfo_queries;
    uint64_t netinfo_time;
    uint64_t hardware_queries;
    uint64_t hardware_time;
    uint64_t osinfo_queries;
    uint64_t osinfo_time;
} agent_syscollector_t;

typedef struct _agent_breakdown_t {
    uint64_t sql_queries;
    uint64_t sql_time;
    uint64_t remove_queries;
    uint64_t remove_time;
    uint64_t begin_queries;
    uint64_t begin_time;
    uint64_t commit_queries;
    uint64_t commit_time;
    uint64_t close_queries;
    uint64_t close_time;
    uint64_t rootcheck_queries;
    uint64_t rootcheck_time;
    uint64_t sca_queries;
    uint64_t sca_time;
    uint64_t ciscat_queries;
    uint64_t ciscat_time;
    uint64_t vulnerability_detector_queries;
    uint64_t vulnerability_detector_time;
    uint64_t dbsync_queries;
    uint64_t dbsync_time;
    uint64_t unknown_queries;

    agent_syscheck_t syscheck;
    agent_syscollector_t syscollector;
} agent_breakdown_t;

typedef struct _global_agent_t {
    uint64_t insert_agent_queries;
    uint64_t insert_agent_time;
    uint64_t update_agent_data_queries;
    uint64_t update_agent_data_time;
    uint64_t update_agent_name_queries;
    uint64_t update_agent_name_time;
    uint64_t update_agent_group_queries;
    uint64_t update_agent_group_time;
    uint64_t update_keepalive_queries;
    uint64_t update_keepalive_time;
    uint64_t update_connection_status_queries;
    uint64_t update_connection_status_time;
    uint64_t reset_agents_connection_queries;
    uint64_t reset_agents_connection_time;
    uint64_t sync_agent_info_set_queries;
    uint64_t sync_agent_info_set_time;
    uint64_t delete_agent_queries;
    uint64_t delete_agent_time;
    uint64_t select_agent_name_queries;
    uint64_t select_agent_name_time;
    uint64_t select_agent_group_queries;
    uint64_t select_agent_group_time;
    uint64_t select_keepalive_queries;
    uint64_t select_keepalive_time;
    uint64_t find_agent_queries;
    uint64_t find_agent_time;
    uint64_t get_agent_info_queries;
    uint64_t get_agent_info_time;
    uint64_t get_all_agents_queries;
    uint64_t get_all_agents_time;
    uint64_t get_agents_by_connection_status_queries;
    uint64_t get_agents_by_connection_status_time;
    uint64_t disconnect_agents_queries;
    uint64_t disconnect_agents_time;
    uint64_t sync_agent_info_get_queries;
    uint64_t sync_agent_info_get_time;
} global_agent_t;

typedef struct _global_group_t {
    uint64_t insert_agent_group_queries;
    uint64_t insert_agent_group_time;
    uint64_t delete_group_queries;
    uint64_t delete_group_time;
    uint64_t select_groups_queries;
    uint64_t select_groups_time;
    uint64_t find_group_queries;
    uint64_t find_group_time;
} global_group_t;

typedef struct _global_belongs_t {
    uint64_t insert_agent_belong_queries;
    uint64_t insert_agent_belong_time;
    uint64_t delete_agent_belong_queries;
    uint64_t delete_agent_belong_time;
    uint64_t delete_group_belong_queries;
    uint64_t delete_group_belong_time;
} global_belongs_t;

typedef struct _global_labels_t {
    uint64_t set_labels_queries;
    uint64_t set_labels_time;
    uint64_t get_labels_queries;
    uint64_t get_labels_time;
} global_labels_t;

typedef struct _global_breakdown_t {
    uint64_t sql_queries;
    uint64_t sql_time;
    uint64_t unknown_queries;

    global_agent_t agent;
    global_group_t group;
    global_belongs_t belongs;
    global_labels_t labels;
} global_breakdown_t;

typedef struct _task_upgrade_t {
    uint64_t upgrade_queries;
    uint64_t upgrade_time;
    uint64_t upgrade_custom_queries;
    uint64_t upgrade_custom_time;
    uint64_t upgrade_get_status_queries;
    uint64_t upgrade_get_status_time;
    uint64_t upgrade_update_status_queries;
    uint64_t upgrade_update_status_time;
    uint64_t upgrade_result_queries;
    uint64_t upgrade_result_time;
    uint64_t upgrade_cancel_tasks_queries;
    uint64_t upgrade_cancel_tasks_time;
} task_upgrade_t;

typedef struct _task_breakdown_t {
    uint64_t sql_queries;
    uint64_t sql_time;
    uint64_t set_timeout_queries;
    uint64_t set_timeout_time;
    uint64_t delete_old_queries;
    uint64_t delete_old_time;
    uint64_t unknown_queries;

    task_upgrade_t upgrade;
} task_breakdown_t;

typedef struct _mitre_breakdown_t {
    uint64_t sql_queries;
    uint64_t sql_time;
    uint64_t unknown_queries;
} mitre_breakdown_t;

typedef struct _wazuhdb_breakdown_t {
    uint64_t remove_queries;
    uint64_t remove_time;
    uint64_t unknown_queries;
} wazuhdb_breakdown_t;

typedef struct _queries_breakdown_t {
    uint64_t wazuhdb_queries;
    uint64_t wazuhdb_time;
    uint64_t agent_queries;
    uint64_t agent_time;
    uint64_t global_queries;
    uint64_t global_time;
    uint64_t task_queries;
    uint64_t task_time;
    uint64_t mitre_queries;
    uint64_t mitre_time;
    uint64_t unknown_queries;

    wazuhdb_breakdown_t wazuhdb_breakdown;
    agent_breakdown_t agent_breakdown;
    global_breakdown_t global_breakdown;
    task_breakdown_t task_breakdown;
    mitre_breakdown_t mitre_breakdown;
} queries_breakdown_t;

typedef struct _db_stats_t {
    char *version;
    char *deamon_name;
    uint64_t timestamp;
    uint64_t queries_total;
    uint64_t queries_time_total;

    queries_breakdown_t queries_breakdown;
} db_stats_t;

void w_inc_queries_total();

void w_inc_wazuhdb();

void w_inc_wazuhdb_remove();

void w_inc_wazuhdb_unknown();

void w_inc_agent();

void w_inc_agent_sql();

void w_inc_agent_remove();

void w_inc_agent_begin();

void w_inc_agent_commit();

void w_inc_agent_close();

void w_inc_agent_rootcheck();

void w_inc_agent_sca();

void w_inc_agent_ciscat();

void w_inc_agent_vul_detector();

void w_inc_agent_dbsync();

void w_inc_agent_unknown();

void w_inc_agent_syscheck();

void w_inc_agent_fim_file();

void w_inc_agent_fim_registry();

void w_inc_agent_syscollector_processes();

void w_inc_agent_syscollector_packages();

void w_inc_agent_syscollector_hotfixes();

void w_inc_agent_syscollector_ports();

void w_inc_agent_syscollector_network_protocol();

void w_inc_agent_syscollector_network_address();

void w_inc_agent_syscollector_network_iface();

void w_inc_agent_syscollector_hwinfo();

void w_inc_agent_syscollector_osinfo();

void w_inc_agent_syscollector_deprecated_process();

void w_inc_agent_syscollector_deprecated_packages();

void w_inc_agent_syscollector_deprecated_hotfixes();

void w_inc_agent_syscollector_deprecated_ports();

void w_inc_agent_syscollector_deprecated_network_protocol();

void w_inc_agent_syscollector_deprecated_network_address();

void w_inc_agent_syscollector_deprecated_network_info();

void w_inc_agent_syscollector_deprecated_hardware();

void w_inc_agent_syscollector_deprecated_osinfo();

void w_inc_global();

void w_inc_global_sql();

void w_inc_global_unknown();

void w_inc_global_agent_insert_agent();

void w_inc_global_agent_update_agent_data();

void w_inc_global_agent_update_agent_name();

void w_inc_global_agent_update_agent_group();

void w_inc_global_agent_update_keepalive();

void w_inc_global_agent_update_connection_status();

void w_inc_global_agent_reset_agents_connection();

void w_inc_global_agent_sync_agent_info_set();

void w_inc_global_agent_delete_agent();

void w_inc_global_agent_select_agent_name();

void w_inc_global_agent_select_agent_group();

void w_inc_global_agent_select_keepalive();

void w_inc_global_agent_find_agent();

void w_inc_global_agent_get_agent_info();

void w_inc_global_agent_get_all_agents();

void w_inc_global_agent_get_agents_by_connection_status();

void w_inc_global_agent_disconnect_agents();

void w_inc_global_agent_sync_agent_info_get();

void w_inc_global_group_insert_agent_group();

void w_inc_global_group_delete_group();

void w_inc_global_group_select_groups();

void w_inc_global_group_find_group();

void w_inc_global_belongs_insert_agent_belong();

void w_inc_global_belongs_delete_agent_belong();

void w_inc_global_belongs_delete_group_belong();

void w_inc_global_labels_set_labels();

void w_inc_global_labels_get_labels();

void w_inc_task();

void w_inc_task_sql();

void w_inc_task_set_timeout();

void w_inc_task_delete_old();

void w_inc_task_unknown();

void w_inc_task_upgrade();

void w_inc_task_upgrade_custom();

void w_inc_task_upgrade_get_status();

void w_inc_task_upgrade_update_status();

void w_inc_task_upgrade_result();

void w_inc_task_upgrade_cancel_tasks();

void w_inc_mitre();

void w_inc_mitre_sql();

void w_inc_mitre_unknown();

void w_inc_unknown();

void w_inc_agent_syscheck_time(struct timeval time);

