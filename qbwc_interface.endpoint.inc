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
class qbwc_interface_webconnector
{
  public function serverVersion() {
    $this->serverVersionResult = '2.0';
    return $this;
  }

  public function clientVersion($clientVersion) {
    $this->clientVersionResult = '';
    return $this;
  }

  public function authenticate($authenticate) {
    $user = variable_get(QBWC_INT_USER);
    $pwd = variable_get(QBWC_INT_PASSWORD);

    if(!($user && ($authenticate->strUserName == $user) &&
      $pwd && ($authenticate->strPassword == $pwd))
    ) {
      // doesn't pass validation
      $message = 'WARNING: qbwc_interface authentication probed with invalid credentials';
      watchdog('QBWC', $message, NULL, 'error');
      $this->authenticateResult[] = '';    // per Intuit QBWC docs
      $this->authenticateResult[] = 'nvu'; // per Intuit QBWC docs
      return $this;
    }

    $ticket = uniqid();
    variable_set(QBWC_INT_TICKET, $ticket); // store session
    $this->authenticateResult[] = $ticket;
    $this->authenticateResult[] = '';

    return $this;
  }

  /**
   * sendRequestXML() is called when the Quickbooks Webconnector is requesting a QBXML query.
   * This is a key entryway to Drupal and is the point at which one of the two hooks provided by the
   * qbwc_interface API gets triggered. This hook, identified by the QBWC_HOOK_SENDREQUEST constant is documented in
   * the qbwc_interface API documentation.
   * This function is the point at which the Drupal Soap server is asked to turn around and behave as a Quickbooks
   * XML client and send a QBXML request back to Quickbooks.
   *
   * @param $sendRequestXML
   * @return $this
   */
  public function sendRequestXML($sendRequestXML) {
    $ticket = variable_get(QBWC_INT_TICKET);

    if($sendRequestXML->strHCPResponse) {
      $company = new SimpleXMLElement($sendRequestXML->strHCPResponse);
      $companyjson = json_encode($company);
      variable_set(QBWC_INT_COMPANY, $companyjson);
    } else {
      $companyjson = variable_get(QBWC_INT_COMPANY);
    }
    $company = json_decode($companyjson);

    if(!($ticket && ($sendRequestXML->ticket == $ticket))) {
      watchdog('QBWC', 'QBWC::sendRequestXML[' .
        $ticket . ']( ' . $sendRequestXML->ticket . ': ' .
        _var_dump2str($company) . ', ' .
        $sendRequestXML->strCompanyFileName . ', ' .
        $sendRequestXML->qbXMLCountry . ', ' . $sendRequestXML->qbXMLMajorVers . '.' . $sendRequestXML->qbXMLMinorVers . ' )', NULL, 'error');
      $this->sendRequestXMLResult = ''; // do nothing
      $lastErr = 'SR:' . QBWC_INT_ERR_TICKET_MISMATCH . // ':' . $ticket .
        ':' . $sendRequestXML->ticket;
      variable_set(QBWC_INT_LASTERR, $lastErr);
      return $this;
    }

    // pass XML request to hook qbwc_sendRequestXML()
    if(sizeof(module_implements(QBWC_HOOK_SENDREQUEST)) > 0) {
      // Call all modules that implement the hook, and let them make changes to $variables.
      $this->sendRequestXMLResult = implode('',
        module_invoke_all(QBWC_HOOK_SENDREQUEST,
          $sendRequestXML->ticket,
          $company,
          $sendRequestXML->strCompanyFileName,
          $sendRequestXML->qbXMLCountry,
          $sendRequestXML->qbXMLMajorVers,
          $sendRequestXML->qbXMLMinorVers)
      );
    } else {
      $this->sendRequestXMLResult = '';
    }
    return $this;
  }

  /**
   * receiveResponseXML() is the second of two access points between Drupal and the Quickbooks Webconnector. The hook
   * identified by the constant QBWC_HOOK_RECEIVERESPONSE is triggered here and is documented within the
   * qbwc_interface API documentation.
   *
   * Once again, this is the point at which the Drupal installation is asked to turn around and behave not as a SOAP
   * server, but as a QBXML client receiving a QBXML response to a previous QBXML query.
   *
   * @param $receiveResponseXML
   * @return $this
   */
  public function receiveResponseXML($receiveResponseXML) {
    $ticket = variable_get(QBWC_INT_TICKET);

    // could fetch company info and json_decode it if it turns out to be needed
    if(!($ticket && ($receiveResponseXML->ticket == $ticket))) {
      watchdog('QBWC', 'QBWC::receiveResponseXML[' . $ticket . ']( ' .
        $receiveResponseXML->ticket . ', ' .
        $receiveResponseXML->strHCPResponse . ', ' .
        $receiveResponseXML->strCompanyFileName . ', ' .
        $receiveResponseXML->qbXMLCountry . ', ' . $receiveResponseXML->qbXMLMajorVers . '.' . $receiveResponseXML->qbXMLMinorVers . ' )', NULL, 'error');
      $this->receiveResponseXMLResult = -1; // per Intuit QBWC docs
      $lastErr = 'RR:' . QBWC_INT_ERR_TICKET_MISMATCH; // ':' . $ticket . ':' . $receiveResponseXML->ticket;
      variable_set(QBWC_INT_LASTERR, $lastErr);
      return $this;
    }

    // send to hook qbwc_receiveResponseXML()
    if(sizeof(module_implements(QBWC_HOOK_RECEIVERESPONSE)) > 0) {
      // Call all modules that implement the hook, and let them make changes to $variables.
      $this->receiveResponseXMLResult = implode('',
        module_invoke_all(QBWC_HOOK_RECEIVERESPONSE,
          $ticket,
          $receiveResponseXML->response,
          $receiveResponseXML->hresult,
          $receiveResponseXML->message));
    } else {
      watchdog('QBWC', 'QBWC:receiveResponseXML-nohook(' . $receiveResponseXML->response . ', ' .
        $receiveResponseXML->hresult . ', ' . $receiveResponseXML->message . ')', NULL, 'error');
      $this->receiveResponseXMLResult = 100;
    }

    $this->receiveResponseXMLResult += 0;
    return $this;
  }

  public function connectionError($connectionError) {
    $ticket = variable_get(QBWC_INT_TICKET);

    watchdog('QBWC', 'QBWC::connectionError[' .
      $ticket . ']( ' .
      $connectionError->ticket . ', ' .
      $connectionError->hresult . ', ' .
      $connectionError->message . ')', NULL, 'error');

    $this->connectionErrorResult = 'done'; // per Intuit QBWC docs
    return $this;
  }

  public function getLastError($getLastError) {
    $ticket = variable_get(QBWC_INT_TICKET);

    if(!($ticket && ($getLastError->ticket == $ticket))) {
      return $this;
    }
    $this->getLastErrorResponse = variable_get(QBWC_INT_LASTERR);
    return $this;
  }

  public function closeConnection($closeConnection) {
    $ticket = variable_get(QBWC_INT_TICKET);

    if(!($ticket && ($closeConnection->ticket == $ticket))) {
      // original ticket got lost. this should get logged even if not debugging
      $errmsg = 'QBWC::closeConnection( ' . $closeConnection->ticket . ' != ' . $ticket . ' )';
      watchdog('QBWC', $errmsg, NULL, 'error');
      $this->closeConnectionResult = 'CC: ' . $closeConnection->ticket;
      return $this;
    }
    $this->closeConnectionResult = 'QBWC::closeConnection( ' . $ticket . ' )';
    variable_set(QBWC_INT_TICKET, 0);

    return ($this);
  }
}

/**
 * Implementation of the interface endpoint.
 *
 */
function qbwc_interface_endpoint() {
  global $base_url;

  try {
    // ini_set("soap.wsdl_cache_enabled", "0"); // turn off cache when changing wsdl file
    $server = new \SoapServer($base_url . '/' . variable_get(QBWC_INT_ENDPT) . '.wsdl');
    $server->setClass('qbwc_interface_webconnector');
    qbwc_interface_ob_reset();
    $server->handle();
  } catch(Exception $e) {
    watchdog('QBWC', 'NOSOAP ' . $e->getMessage(), NULL, 'error');
  }
  exit;
}

