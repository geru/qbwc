<?php

/**
 * @file
 * Implements PHP::soapserver class to interact with Quickbooks Web Connector
 *
 * Two API hooks:
 * qbwc_sendRequestXML
 * qbwc_receiveResponseXML
 *
 */

function _qbwc_get_connection_by_ticket($endpoint, $ticket = NULL) {
  return ($ticket
    ? db_select('qbwc', 'c')
      ->condition('c.endpoint', $endpoint)
      ->condition('c.ticket', $ticket)
      ->fields('c')
      ->execute()
      ->fetchAssoc()
    : NULL
  );
}

function _qbwc_int_convert( $unsigned ) {
  if (PHP_INT_SIZE == 8)
  {
    if ($unsigned>0x7FFFFFFF)
    {
      $unsigned -= 0x100000000;
    }
  }
  return $unsigned;
}

class qbwc_webconnector {
  public function serverVersion() {
    $this->serverVersionResult = '2.0';
    return $this;
  }

  public function clientVersion($clientVersion) {
    $this->clientVersionResult = '';
    return $this;
  }

  public function authenticate($authenticate) {
    $endpoint = $_GET['q'];
    // get $endpoint $username $password from qbwc as authorized connection
    $query = db_select('qbwc', 'e')
      ->condition('e.endpoint', $endpoint)
      ->condition('e.qwc_username', $authenticate->strUserName)
      ->condition('e.password', $authenticate->strPassword)
      ->fields('e')
      ->execute();
    if (!$query->rowCount()) {
      // doesn't pass validation
      watchdog('QBWC', 'WARNING: endpoint(' . $endpoint . ') authentication probed with invalid credentials (' . 
        $authenticate->strUserName . ':' . 
        $authenticate->strPassword . ')',
        NULL, 'error');
      $this->authenticateResult[] = '';    // per Intuit QBWC docs
      $this->authenticateResult[] = 'nvu'; // per Intuit QBWC docs
    }
    else {
      $ticket = qbwc_GUID();
      db_update('qbwc')
        ->fields(array('ticket' => $ticket))
        ->condition('endpoint', $endpoint)
        ->condition('qwc_username', $authenticate->strUserName)
        ->condition('password', $authenticate->strPassword)
        ->execute();
      $this->authenticateResult[] = $ticket;
      $this->authenticateResult[] = '';
    }
    return $this;
  }

  /**
   * sendRequestXML() is called when the Quickbooks Webconnector is requesting a
   * QBXML query. This is a key entryway to Drupal and is the point at which one
   * of the two hooks provided by the qbwc API gets triggered. This hook,
   * identified by the string 'qbwc_sendRequestXML' is documented in the qbwc
   * API documentation.
   *
   * This function is the point at which the Drupal Soap server is asked to turn
   * around and behave as a Quickbooks XML client and send a QBXML request back
   * to Quickbooks.
   *
   * @param $sendRequestXML
   * @return $this
   */
  public function sendRequestXML($sendRequestXML) {
    $endpoint = $_GET['q'];
    $ticket = $sendRequestXML->ticket;
    if (!($connection = _qbwc_get_connection_by_ticket($endpoint, $ticket))) {
      // $error = QBWC_INT_ERR_TICKET_MISMATCH;
      watchdog('QBWC', 'sendRequestXML(' . $endpoint . ')[' . $ticket . ']( ' .
        $sendRequestXML->ticket . ': ' .
        $sendRequestXML->strCompanyFileName . ', ' .
        $sendRequestXML->qbXMLCountry . ', ' .
        $sendRequestXML->qbXMLMajorVers . '.' .
        $sendRequestXML->qbXMLMinorVers . ' )',
        NULL, 'error');
      $this->sendRequestXMLResult = ''; // do nothing
      return $this;
    }
    $connection['data'] = json_decode($connection['data']);
    if ($sendRequestXML->strHCPResponse) {
      $response = new SimpleXMLElement($sendRequestXML->strHCPResponse);
      $data['host'] = $response->QBXMLMsgsRs->HostQueryRs->HostRet;
      $data['company'] = $response->QBXMLMsgsRs->CompanyQueryRs->CompanyRet;
      unset($data['company']->DataExtRet); // changes each time
      $data['preferences'] = $response->QBXMLMsgsRs->PreferencesQueryRs->PreferencesRet;
      $jsondata = json_encode($data);
      $hash = _qbwc_int_convert( crc32( $jsondata ) );
      if( $connection['datahash'] != $hash ) {
        db_update('qbwc')
          ->condition('endpoint', $endpoint)
          ->condition('ticket', $ticket)
          ->fields( array('data' => $jsondata, 'datahash' => $hash))
          ->execute();
        module_invoke_all( 'qbwc_companyInfo', $connection['bookid'], $data );
      }
    }
    
    // pass XML request to hook qbwc_sendRequestXML()
    if (sizeof(module_implements('qbwc_sendRequestXML')) > 0) {
      // Call all modules that implement the hook, and let them make changes to $variables.
      $ret = implode('', module_invoke_all('qbwc_sendRequestXML', $connection['bookid'], $sendRequestXML));
      $this->sendRequestXMLResult = $ret ?
        '<?xml version="1.0"?><?qbxml version="' . $connection['qbxml_version'] . '"?>' . $ret :
        $ret;
    }
    else {
      $this->sendRequestXMLResult = '';
    }
    return $this;
  }

  /**
   * receiveResponseXML() is the second of two access points between Drupal and
   * the Quickbooks Webconnector. The hook identified by the string
   * 'qbwc_receiveResponseXML' is triggered here and is documented within the
   * qbwc API documentation.
   *
   * Once again, this is the point at which the Drupal installation is asked to
   * turn around and behave not as a SOAP server, but as a QBXML client
   * receiving a QBXML response to a previous QBXML query.
   *
   * @param $receiveResponseXML
   * @return $this
   */
  public function receiveResponseXML($receiveResponseXML) {
    $endpoint = $_GET['q'];
    $ticket = $receiveResponseXML->ticket;
    if (!($connection = _qbwc_get_connection_by_ticket($endpoint, $ticket))) {
      // $error = QBWC_INT_ERR_TICKET_MISMATCH;
      watchdog('QBWC', 'QBWC::receiveResponseXML bad ticket: ' .
        $receiveResponseXML->ticket . ', ' .
        $receiveResponseXML->strHCPResponse . ', ' .
        $receiveResponseXML->strCompanyFileName . ', ' .
        $receiveResponseXML->qbXMLCountry . ', ' .
        $receiveResponseXML->qbXMLMajorVers . '.' .
        $receiveResponseXML->qbXMLMinorVers . ' )',
        NULL, 'error');
      $this->receiveResponseXMLResult = -1; // per Intuit QBWC docs
      return $this;
    }

    $connectiondata = json_decode($connection['data']);

    // send to hook qbwc_receiveResponseXML()
    if (sizeof(module_implements('qbwc_receiveResponseXML')) > 0) {
      // Call all modules that implement the hook
      $values = module_invoke_all('qbwc_receiveResponseXML',
        $connection['bookid'], $receiveResponseXML, $connectiondata);
      $ret = 0; // this next loop is silly. Only one module should process the information and return a value. The loop checks in case multiple modules did it and tries to make sense of the results
      foreach( $values as $value ) {
        if( is_int($value) ) {
          $ret += $value;
        }
      }
      $ret = ($ret > 100) ? 100 : ($ret < 0 ? 0 : $ret);
      $this->receiveResponseXMLResult = $ret;
    }
    else {
      watchdog('QBWC', 'QBWC:receiveResponseXML-nohook(' .
        $receiveResponseXML->response . ', ' .
        $receiveResponseXML->hresult . ', ' .
        $receiveResponseXML->message . ')',
        NULL, 'error');
      $this->receiveResponseXMLResult = 100;
    }

    $this->receiveResponseXMLResult += 0;
    return $this;
  }

  public function connectionError($connectionError) {
    watchdog('QBWC', 'QBWC::connectionError[' .
      $connectionError->ticket . ']( ' .
      $connectionError->hresult . ', ' .
      $connectionError->message . ')', NULL, 'error');

    $this->connectionErrorResult = 'done'; // per Intuit QBWC docs
    return $this;
  }

  public function getLastError($getLastError) {
    if ($connection = _qbwc_get_connection_by_ticket($_GET['q'], $getLastError->ticket)) {
      $this->getLastErrorResponse = $connection['lasterr'];
    }
    return $this;
  }

  // Implements QBWC closeConnection: see SDK documentation
  public function closeConnection($closeConnection) {
    $this->closeConnectionResult = '';

    if ($connection = _qbwc_get_connection_by_ticket($_GET['q'], $closeConnection->ticket)) {
      if (sizeof(module_implements('qbwc_closeConnection')) > 0) {
        // Call all modules that implement the close connection hook
        $this->closeConnectionResult = implode('',
          module_invoke_all('qbwc_closeConnection', $connection['bookid'], $closeConnection));
      }
      db_update('qbwc')->fields(array('ticket' => NULL))
        ->condition('endpoint', $connection['endpoint'])
        ->condition('ticket', $closeConnection->ticket)
        ->execute();
    }
    else {
      watchdog('QBWC', 'QBWC::closeConnection bad ticket ' . $closeConnection->ticket, NULL, 'error');
    }

    return ($this);
  }
}

/**
 * Implementation of the interface endpoint.
 *
 */
function qbwc_endpoint() {
  global $base_url;

  try {
    // ini_set("soap.wsdl_cache_enabled", "0"); // turn off cache when changing wsdl file
    $server = new \SoapServer($base_url . '/' . $_GET['q'] . '.wsdl');
    $server->setClass('qbwc_webconnector');
    qbwc_ob_reset();
    $server->handle();
  } catch(Exception $e) {
    watchdog('QBWC', 'NOSOAP ' . $e->getMessage(), NULL, 'error');
  }
  exit;
}

