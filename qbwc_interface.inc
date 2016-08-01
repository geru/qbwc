<?php
// $Id:$

/**
 * @file
 * QBWC Interface Definitions, errors and Drupal variables
 */

// namespace Drupal\qbwc_interface;
// use Drupal\qbwc_interface;

// constants
define('QBWC_INT_ERR_TICKET_MISMATCH', '33');
define('QBWC_INT_TTL', 600); // seconds for apc_ calls timeout or lose data
define('QBWC_HOOK_SENDREQUEST', 'qbwc_sendRequestXML');
define('QBWC_HOOK_RECEIVERESPONSE', 'qbwc_receiveResponseXML');

// drupal variables storing key values
define('QBWC_INT_OWNERID', 'qbwc_interface_ownerid');
define('QBWC_INT_FILEID', 'qbwc_interface_fileid');
define('QBWC_INT_USER', 'qbwc_interface_username');
define('QBWC_INT_PASSWORD', 'qbwc_interface_password');
define('QBWC_INT_READONLY', 'qbwc_interface_readonly');
define('QBWC_INT_AUTHFLAGS', 'qbwc_interface_authflags');
define('QBWC_INT_QBTYPE', 'qbwc_interface_qbtype');
define('QBWC_INT_ENDPT', 'qbwc_interface_endpoint');

// PHP apc variables
define('QBWC_INT_LASTERR', 'qbwc_interface_lasterror');
define('QBWC_INT_TICKET', 'qbwc_interface_ticket');
define('QBWC_INT_COMPANY', 'qbwc_interface_company');

