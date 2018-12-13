# -*- coding: utf-8 -*-
"""
Created on May 2018

@authors: Alexis BOGROFF
contacts: alexis.bogroff@gmail.com

Class for handling interactions with SQL type databases (PostGreSQL, MySQL, Access)
"""

import psycopg2
import pandas as pd
import datetime

class ClassSqlDb:
    """
    Object for handling SQL databases (PostGre) core operations

    Attributes:
        - __att_db_name
        - __att_db_user
        - __att_db_password
        - __att_db_host
        - __att_db_connection: connection obj with the database
    Methods:
        - __init__: set credentials and establish connection
        - __execute_request: execute request while handling cursor management
        - method_commit: saves changes (inserts, deletes)
        - method_connect: established connection with a PostGre table
        - method_connection_close: closes connection and/or saves changes
        - method_delete: delete a row
        - method_get_connection_state
        - method_get_execution_state: provide info on asynchronous processes
        - method_insert: inserts one/multiple rows
        - method_select: selects data through an sql-like pattern
        - method_table_description: get structure of table (columns, type, ...)
    """

    def __init__(self, db_name, db_user, db_password, db_host):
        """
        Initialize SqlDb object and connect to a PostGre Table
        """
        self.__att_db_host = db_host
        self.__att_db_name = db_name
        self.__att_db_password = db_password
        self.__att_db_user = db_user
        self.__att_db_connection = None

        self.method_connect()  # Initialize connection


    def __del__(self):
        """
        Close the connection while deleting the object
        """
        self.method_connection_close()


    def __execute_request(self, str_request, str_return_values=None, int_batch_size=2000,
                          bool_server_cursor=False):
        """
        Execute main requests while handling transaction management (open and close cursor)
        str_return_values:
            - None
            - all
            - batch
        If batch selected, specify their size using int_batch_size e.g.=100000 (default: 2000)
        the default value of 2000 allows to fetch about 100KB per roundtrip assuming records
        of 10-20 columns of mixed number and strings; you may decrease this value if you are dealing with huge records.
        see: http://initd.org/psycopg/docs/usage.html#server-side-cursors
        see: https://stackoverflow.com/questions/17199113/psycopg2-leaking-memory-after-large-query

        Nb: bool_server_cursor must be set to TRUE for batch to be efficient,
        otherwise the whole data is retrieved in 1 step
        """
        result = []

        # Use a server-side cursor (for heavy loads, minimize client memory usage)
        if bool_server_cursor:
            cur_name = 'cur_name'  # any name would make it
        else:
            cur_name = None

        # Pull Request
        with self.__att_db_connection.cursor(cur_name) as cur:

            if str_return_values is None:
                cur.execute(str_request)

            elif str_return_values == 'all':
                cur.execute(str_request)
                result = cur.fetchall()

            # TODO (alexis stagiaire3@pleiade-am.com) To test on heavy loads (and compare with fetchmany with itzesize)
            # And verify the memory usage, to see if batches actually decrease it since
            # the whole request is not pull at once
            # Nb: cur.itersize = int_batch_size  # seems to have no effect
            elif str_return_values == 'batch':
                cur.execute(str_request)
                while True:
                    batch = cur.fetchmany(size=int_batch_size)
                    if batch:
                        for row in batch:
                            result.append(row)
                        #print('\n',batch)
                    else:
                        break

#                # supposed to do the same on the server-side in a pythonic way
#                cur.itersize = int_batch_size
#                cur.execute(str_request)
#                for batch in cur:
#                    print("\n", batch)
        return result


    def method_commit(self):
        """
        Commit transactions to the dataframe in order to save inserts, updates and deletes
        If no commit is made before destructing the object, no change will be saved
        Find the right balance between committing frequently enough to release memory etc.
        and not committing too frequently to prevent from slowing down the process
        """
        self.__att_db_connection.commit()


    def method_connect(self):
        """
        Establish connection with PostGre database given the provided credentials
        """
        # Establish connection
        self.__att_db_connection = psycopg2.connect("dbname=" + self.__att_db_name
                                                    + " user=" + self.__att_db_user
                                                    + " password=" + self.__att_db_password
                                                    + " host=" + self.__att_db_host)


    def method_connection_close(self, bool_save_changes=False, bool_verbose=True):
        """
        Close connections with the database
        """
        if bool_save_changes:  # if enabled, save modifications
            self.method_commit()

        try:  # close connection
            self.__att_db_connection.close()
            if bool_verbose:
                print("\n\nDb sucessfully saved and closed")
        except:
            print("\n\nConnection already closed")


    def method_delete(self, str_delete_table, str_delete_where, bool_verbose=True,
                      str_advanced_request=None):
        """
        Delete specified data

        'Ex ---------------------------------------
        STR_DELETE_FROM_TABLE = 'trades'
        DIC_DELETE_WHERE = "NUMERO_ORDRE = 14"

        OBJ_DB.method_delete(STR_DELETE_FROM_TABLE, DIC_DELETE_WHERE)
        -------------------------------------------
        """
        if not str_advanced_request:
            str_request = "DELETE FROM " + str_delete_table
            str_request += " WHERE " + str_delete_where

            if bool_verbose:
                print('\nSuccess: '+str_request)
        else:
            str_request = str_advanced_request

        self.__execute_request(str_request)


    def method_get_connection_state(self):
        """
        Get current connection' state (connected/disconnected)
        """
        return (self.__att_db_connection.closed == 0)*'Connected' \
                + (self.__att_db_connection.closed != 0)*'Disconnected'


    def method_get_parameters(self):
        """
        Get the specified parameters at initialization
        """
        return {'dbname':self.__att_db_name,
                'user':self.__att_db_user,
                'password':self.__att_db_password,
                'host':self.__att_db_host}


    def method_get_execution_state(self):
        """
        Get current asynchronous execution' state, i.e. if asynchronous op are being executed
        Nb: not to confound with a loop of requests running
        """
        return (self.__att_db_connection.isexecuting() is True)*'Busy' \
                + (not self.__att_db_connection.isexecuting() is True)*'All asynchronous op ended'


    def method_insert(self, str_insert_table, fields_values,
                      bool_verbose=True, str_advanced_request=None):
        """
        Method to insert data in a defined table (SQL)
        Reconstrcut an SQL request
        e.g. "INSERT INTO trades(NUMERO_ORDRE,ISIN) VALUES(16,'FR0000032526')"

        Light Mode: fill a list of dictionnaries in fields_values (better visually)
        Heavy Load Mode: fill a dataframe in fields_values (faster)

        Ex1: -----------------------------------------------------------
        Using a list of dictionaries:
            STR_INSERT_TABLE = 'trades'
            fields_values = [{'date_saisie' : '29/03/2018',
                              'isin'        : 'FR0000048229',
                              'numero_ordre': '30',
                              'sens'        : 'ACHAT'},
                             {'date_saisie' : '30/03/2018',
                              'isin'        : 'FR0000032528',
                              'numero_ordre': '31',
                              'sens'        : 'VENTE'}]

        Using a pandas dataframe:
            STR_INSERT_TABLE = 'trades'
            fields_values = pd.DataFrame([{'date_saisie' : '29/03/2018',
                                          'isin'        : 'FR0000048229',
                                          'numero_ordre': '30',
                                          'sens'        : 'ACHAT'},
                                         {'date_saisie' : '30/03/2018',
                                          'isin'        : 'FR0000032528',
                                          'numero_ordre': '31',
                                          'sens'        : 'VENTE'}])
        -----------------------------------------------------------------
        """
        if not str_advanced_request:

            # ------ LIST OF DICS ------
            if isinstance(fields_values, list):
                # Build 'INSERT INTO' part
                str_request = "INSERT INTO " + str_insert_table + "("
                # Use fields of the first dict in fields_values
                # as they must be identical intto following dicts
                str_request += ", ".join(f'"{field.lower()}"' for field in fields_values[0])

                # Build 'VALUES' part
                str_request += ") VALUES"
                for j, dic in enumerate(fields_values):
                    str_request += " ("
                    for i, field in enumerate(dic):
                        if not dic[field] or dic[field] == "NULL":
                            str_request += "NULL"
                        elif isinstance(dic[field], datetime.datetime):
                            str_request += "TIMESTAMP "
                            str_request += "'" + str(dic[field]) + "'"
                        else:
                            str_request += "'" + str(dic[field]) + "'"
                        if i < len(dic)-1:
                            str_request += ", "
                    str_request += ")"
                    if j < len(fields_values)-1:
                        str_request += ", "

            # ------ DATAFRAME ------
            elif isinstance(fields_values, pd.core.frame.DataFrame):
                # Build 'INSERT INTO' part
                str_request = "INSERT INTO " + str_insert_table + "("
                for i, field in enumerate(fields_values):
                    str_request += field
                    if i < fields_values.shape[1]-1:
                        str_request += ", "

                # Build 'VALUES' part
                str_request += ") VALUES"
                # Iterate over all rows
                for j in range(fields_values.shape[0]):
                    str_request += ' ('
                    for i, field in enumerate(fields_values):
                        if not fields_values[field][j] or fields_values[field][j] == "NULL":
                            str_request += "NULL"
                        elif isinstance(fields_values[field][j], pd._libs.tslibs.timestamps.Timestamp):
                            str_request += "TIMESTAMP "
                            str_request += "'" + str(fields_values[field][j]) + "'"
                        else:
                            str_request += "'" + str(fields_values[field][j]) + "'"
                        # if smaller than number of columns
                        if i < fields_values.shape[1]-1:
                            str_request += ", "
                    str_request += ")"
                    if j < fields_values.shape[0]-1:
                        str_request += ", "

            else:
                raise ValueError("Wrong argument specified in 'fields_values'. " \
                                 "Provided '{}', but should be one of 'list' (of dicts), " \
                                 "or 'pandas.core.frame.DataFrame'." \
                                 .format(type(fields_values)))

        # ------ ADVANCED ------
        else:
            str_request = str_advanced_request

        if bool_verbose:
            print(str_request[0:100])

        # Execute insert
        self.__execute_request(str_request)


    def method_select(self, list_select_args=None, list_from_table=None, list_dics_where_args=None,
                      str_join_args=None, list_dics_orderby_args=None, str_advanced_request=None,
                      str_return_values='all', int_batch_size=2000, bool_server_cursor=False,
                      bool_verbose=False):
        """
        Method to select data from a defined table under specified conditions
        Reconstrcut an SQL request e.g.:
        "SELECT "'"NUMERO_ORDRE"'", "'"ISIN"'" from trades WHERE "'"SENS"'" = '"'ACHAT'"'"

        Inputs:
            - list_select_args: list of string arguments for the select part
            - list_from_args: idem for 'from'
            - list_dics_where_args: idem for 'where'
            - list_dics_orderby_args: list of dictionaries (see Ex1)
                -> where isASC in {1, 0, 'ASC' or 'DESC'}
                -> and ASC <=> 1, DESC <=> 0
            - bool_advanced_request: enables a request to be built completly manually (as string)

        Return:
            - the selected data

        Nb: Single quotes must be used for using values

        --------------------------------------------------------------
        Ex1: select from where orderby
        # Define arguments
        LIST_SELECT_ARGS = ['trades.numero_ordre',
                            'trades.isin',
                            'trades.adresse']
        LIST_FROM_TABLE = ['trades']
        LIST_DICS_WHERE = [
                           {'field':'trades.isin',
                            'operator':'=',
                            'value':'FR0000032526',
                            'link':'AND'},

                           {'field':'trades.numero_ordre',
                            'operator':'>',
                            'value':'18'}
                          ]
        LIST_DICS_ORDERBY_ARGS = [
                                  {'field':'sens',
                                   'direction':'ASC'},
                                  {'field':'numero_ordre',
                                   'direction':'DESC'}
                                 ]

        # Call select method
        result = OBJ_DB.method_select(LIST_SELECT_ARGS, LIST_FROM_ARGS, ...)

        Ex2: select from inner join
        LIST_SELECT_ARGS=['*']
        LIST_FROM_TABLE='trades'
        STR_JOIN_ARGS='inner join table_join on trades.numero_ordre = table_join.numero_ordre'
        OBJ_DB.method_select(list_select_args=LIST_SELECT_ARGS, list_from_table=...)

        Ex3: advanced use
        STR_ADV_REQUEST = '<advanced request>'
        OBJ_DB.method_select(str_advanced_request=STR_ADV_REQUEST)
        --------------------------------------------------------------
        """
        if not str_advanced_request:
            # Add arguments of SELECT
            str_request = "SELECT "
            for i, field in enumerate(list_select_args):
                str_request += field
                if i < len(list_select_args)-1:
                    str_request += ", "

            # Add arguments of FROM
            str_request += " FROM "
            for i, table in enumerate(list_from_table):
                str_request += table
                if i < len(list_from_table)-1:
                    str_request += ", "

            # Add Join arguments
            if str_join_args:
                str_request += ' ' + str_join_args

            # Add arguments of WHERE
            if list_dics_where_args:
                str_where = " WHERE "
                for i, args in enumerate(list_dics_where_args):
                    str_where += args['field'] + ' ' \
                                + args['operator'] + ' ' \
                                + "'" + args['value'] + "'"
                    if i < len(list_dics_where_args)-1:
                        str_where += ' ' + args['link'] + ' '

                str_request += str_where



            # Add arguments of ORDERBY
            if list_dics_orderby_args:
                # Enforce ASC or DESC if 1 or 0 are provided
                for i, dic in enumerate(list_dics_orderby_args):
                    if dic['isASC'] == 1:
                        list_dics_orderby_args[i]['isASC'] = 'ASC'
                    elif dic['isASC'] == 0:
                        list_dics_orderby_args[i]['isASC'] = 'DESC'
                    elif dic['isASC'].upper() == 'ASC':
                        list_dics_orderby_args[i]['isASC'] = 'ASC'
                    elif dic['isASC'].upper() == 'DESC':
                        list_dics_orderby_args[i]['isASC'] = 'DESC'
                    else:
                        raise ValueError("Wrong argument specified in 'ORDER BY'. " \
                                         "Provided: '{}', but should be one of ASC, DESC, 1 or 0" \
                                         .format(dic['isASC']))

                str_orderby = " ORDER BY "
                for i, args in enumerate(list_dics_orderby_args):
                    str_orderby += args['field'] + ' ' + args['isASC']
                    if i < len(list_dics_orderby_args)-1:
                        str_orderby += ", "
                str_request += str_orderby

        else:
            # If a complete request has been manually enforced
            str_request = str_advanced_request

        if bool_verbose:
            print(str_request)

        # Return results
        return self.__execute_request(str_request,
                                      str_return_values=str_return_values,
                                      int_batch_size=int_batch_size,
                                      bool_server_cursor=bool_server_cursor)


    def method_table_description(self, str_table_name, bool_data_type=True,
                                 bool_char_max_length=True):
        """
        Get fields and specifications of a specified table

        inputs:
            - str_table_name: table to describe
            - bool_data_type: get the type of field values
            - bool_char_max_length: get the max length of field' values

        Return:
            - description of the struture
        """
        list_description = []
        str_request = "SELECT column_name"

        if bool_data_type:
            str_request += ", data_type"
        if bool_char_max_length:
            str_request += ", character_maximum_length"

        str_request += " from INFORMATION_SCHEMA.COLUMNS where table_name = ""'" \
                    + str_table_name + "'"

        for field in self.__execute_request(str_request, str_return_values='all'):
            list_description.append(field)

        return list_description


    def method_update(self, str_update_table=None, dic_what=None,
                      list_dics_where=None, str_advanced_request=None, bool_verbose=True):
        """
        Update existing data

        # Ex ----------------------------------------
        STR_UPDATE_TABLE = 'trades'
        DIC_WHAT = {'isin':'FR0000044444',
                     'px_last':'15.2'}
        LIST_DICS_WHERE = [
                          {'field':'trades.isin',
                           'operator':'=',
                           'value':'FR0000032526',
                           'link':'AND'},

                          {'field':'trades.numero_ordre',
                           'operator':'>=',
                           'value':'20'}
                         ]
        ---------------------------------------------
        """
        if not str_advanced_request:
            # Arguments of UPDATE
            str_request = "UPDATE " + str_update_table

            # Arguments of SET
            str_set = ' SET '
            i = 1
            for field in dic_what:
                if not dic_what[field] or dic_what[field] == "NULL":
                    str_set += field + ' = ' \
                                + "NULL"
                elif isinstance(dic_what[field], pd._libs.tslibs.timestamps.Timestamp):
                    str_set += field + ' = ' \
                                + "TIMESTAMP '" + str(dic_what[field]) + "'"
                else:
                    str_set += field + ' = ' \
                                + "'" + str(dic_what[field]) + "'"
                i += 1
                if i <= len(dic_what):
                    str_set += ', '

            str_request += str_set

            # Add arguments of WHERE
            if list_dics_where:
                str_where = " WHERE "
                for i, args in enumerate(list_dics_where):
                    str_where += args['field'] + ' ' \
                                + args['operator'] + ' ' \
                                + "'" + args['value'] + "'"
                    if i < len(list_dics_where)-1:
                        str_where += ' ' + args['link'] + ' '

                str_request += str_where

            if bool_verbose:
                print(str_request)
        else:
            str_request = str_advanced_request

        # Execute update
        self.__execute_request(str_request)
