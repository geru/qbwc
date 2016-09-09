<?php
/**
 * @file
 * QBWC Interface Definitions, errors and Drupal variables
 */

// todo: namespace Drupal\qbwc_interface;
// todo: use Drupal\qbwc_interface;
//
// constants
const QBWC_INT_ERR_TICKET_MISMATCH = '33';
const QBWC_HOOK_SENDREQUEST = 'qbwc_sendRequestXML';
const QBWC_HOOK_RECEIVERESPONSE = 'qbwc_receiveResponseXML';

// drupal variables storing key values
const QBWC_INT_OWNERID = 'qbwc_interface_ownerid';
const QBWC_INT_FILEID = 'qbwc_interface_fileid';
const QBWC_INT_USER = 'qbwc_interface_username';
const QBWC_INT_PASSWORD = 'qbwc_interface_password';
const QBWC_INT_READONLY = 'qbwc_interface_readonly';
const QBWC_INT_AUTHFLAGS = 'qbwc_interface_authflags';
const QBWC_INT_QBTYPE = 'qbwc_interface_qbtype';
const QBWC_INT_ENDPT = 'qbwc_interface_endpoint';

// SOAP error reporting variables
const QBWC_INT_LASTERR = 'qbwc_interface_lasterror';
const QBWC_INT_TICKET = 'qbwc_interface_ticket';
const QBWC_INT_COMPANY = 'qbwc_interface_company';
