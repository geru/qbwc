<?php
/**
 * @file
 * QBWC Interface Definitions, errors and Drupal variables
 */

// todo: namespace Drupal\qbwc_interface;
// todo: use Drupal\qbwc_interface;
//

const QBWC_INTERFACE_MENUROUTE = 'admin/enterprise/qb/qbwc';
const QBWC_WSDL = 'qbwc.wsdl';

// constants
const QBWC_INT_ERR_TICKET_UNKNOWN = '32';
const QBWC_INT_ERR_TICKET_MISMATCH = '33';
const QBWC_INT_ERR_TICKET_NULL = '34';
const QBWC_HOOK_SENDREQUEST = 'qbwc_sendRequestXML';
const QBWC_HOOK_RECEIVERESPONSE = 'qbwc_receiveResponseXML';

