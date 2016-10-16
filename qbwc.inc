<?php
/**
 * @file
 * QBWC Interface Definitions, errors and Drupal variables
 */

// todo: namespace Drupal\qbwc;
// todo: use Drupal\qbwc;
//

const QBWC_MENUROUTE = 'admin/config/enterprise';
const QBWC_EXT_WSDL = '.wsdl';

// constants
const QBWC_INT_ERR_TICKET_UNKNOWN = '32';
const QBWC_INT_ERR_TICKET_MISMATCH = '33';
const QBWC_INT_ERR_TICKET_NULL = '34';

// defined hooks
const QBWC_HOOK_SENDREQUEST = 'qbwc_sendRequestXML';
const QBWC_HOOK_RECEIVERESPONSE = 'qbwc_receiveResponseXML';

