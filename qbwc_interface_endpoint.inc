<?php
// $Id:$

require_once 'qbwc_interface.inc';

/*
 * Implements PHP::soapserver class to interact with Quickbooks Web Connector
 *
 * Two API hooks:
 * qbwc_sendRequestXML
 * qbwc_receiveResponseXML
 *
 */

class qbwc_interface_webconnector
{

  public function serverVersion() { //$serverVersion) {
    $ret = new \stdClass();
    $ret->serverVersionResult = '2.0';
    return $ret;
  }

  public function clientVersion($clientVersion) {
    // watchdog('QBWC', 'clientVersion( ' . $clientVersion->strVersion . ' )');
    $ret = new \stdClass();
    $ret->clientVersionResult = '';
    return $ret;
  }


  // user is set on setup form and must have correct role
  public function authenticate($authenticate) {
    $user = variable_get(QBWC_INT_USER);
    $pwd = variable_get(QBWC_INT_PASSWORD);
    // watchdog('QBWC', 'authenticate( ' . $authenticate->strUserName . ', ' . $authenticate->strPassword . ' )');
    $ret = new \stdClass();


    if (!($user && ($authenticate->strUserName == $user) &&
      $pwd && ($authenticate->strPassword == $pwd))
    ) {
      // doesn't pass validation
      $message = 'WARNING: qbwc_interface authentication probed with invalid credentials';
      watchdog( 'QBWC', $message, NULL, 'error' );
      $ret->authenticateResult[] = '';    // per Intuit QBWC docs
      $ret->authenticateResult[] = 'nvu'; // per Intuit QBWC docs
      return $ret;
    }

    // user_module_invoke('login', $edit, $user);
    $ticket = uniqid();
    apc_store(QBWC_INT_TICKET, $ticket, QBWC_INT_TTL); // store session
    $ret->authenticateResult[] = $ticket;
    $ret->authenticateResult[] = '';

    return $ret;
  }

  // passes response results to hook_sendRequestXML
  public function sendRequestXML($sendRequestXML) {
    $ret = new \stdClass();
    $ticket = apc_fetch(QBWC_INT_TICKET);

    // watchdog( 'QBWC', 'Point 1 (' . $sendRequestXML->strHCPResponse . ')' );
    if ($sendRequestXML->strHCPResponse) {
      $company = new SimpleXMLElement($sendRequestXML->strHCPResponse);
      $companyjson = json_encode($company);
      apc_store(QBWC_INT_COMPANY, $companyjson, QBWC_INT_TTL);
      // watchdog( 'QBWC', 'Point 2 (' . _var_dump2str(self::$company) . ')' );
      // apc_store( QBWC_INT_COMPANY, (array) self::$company, QBWC_INT_TTL );
    }
    else {
      $companyjson = apc_fetch(QBWC_INT_COMPANY);
    }
    $company = json_decode($companyjson);

    // watchdog('QBWC', 'Point 4');
    if (!($ticket && ($sendRequestXML->ticket == $ticket))) {
      watchdog('QBWC', 'QBWC::sendRequestXML[' .
        $ticket . ']( ' . $sendRequestXML->ticket . ': ' .
        _var_dump2str($company) . ', ' .
        $sendRequestXML->strCompanyFileName . ', ' .
        $sendRequestXML->qbXMLCountry . ', ' . $sendRequestXML->qbXMLMajorVers . '.' . $sendRequestXML->qbXMLMinorVers . ' )', NULL, 'error');
      $ret->sendRequestXMLResult = ''; // do nothing
      $lastErr = 'SR:' . QBWC_INT_ERR_TICKET_MISMATCH . // ':' . $ticket .
        ':' . $sendRequestXML->ticket;
      apc_store(QBWC_INT_LASTERR, $lastErr, QBWC_INT_TTL);
      return $ret;
    }

    // passes XML request to hook qbwc_sendRequestXML()
    if (sizeof(module_implements(QBWC_HOOK_SENDREQUEST)) > 0) {
      // watchdog('QBWC', 'Point 5');
      // Call all modules that implement the hook, and let them make changes to $variables.
      $ret->sendRequestXMLResult = implode('',
        module_invoke_all(QBWC_HOOK_SENDREQUEST,
          $sendRequestXML->ticket,
          $company,
          $sendRequestXML->strCompanyFileName,
          $sendRequestXML->qbXMLCountry,
          $sendRequestXML->qbXMLMajorVers,
          $sendRequestXML->qbXMLMinorVers)
      );
      // watchdog('QBWC', 'QBWC:sendRequestXML-hook[' . $ticket . ']( ' .
      //  _var_dump2str($company) . ', ' .
      //  $sendRequestXML->strCompanyFileName . ', ' .
      //  $sendRequestXML->qbXMLCountry . ', ' . $sendRequestXML->qbXMLMajorVers . '.' . $sendRequestXML->qbXMLMinorVers . ' )=(' .
      //  htmlspecialchars($ret->sendRequestXMLResult) . ')'
      // );
    }
    else {
      // watchdog('QBWC', 'QBWC::sendRequestXML-nohook[' . $ticket . ']( ' .
      //  _var_dump2str($company) . ', ' .
      //  $sendRequestXML->strCompanyFileName . ', ' .
      //  $sendRequestXML->qbXMLCountry . ', ' . $sendRequestXML->qbXMLMajorVers . '.' . $sendRequestXML->qbXMLMinorVers . ' )');
      $ret->sendRequestXMLResult = '';
    }

// watchdog( 'QBWC', 'Point 6' );
    return $ret;
  }


  // passes response results to hook_receiveResponseXML
  public function receiveResponseXML($receiveResponseXML) {
    $ret = new \stdClass();
    $ticket = apc_fetch(QBWC_INT_TICKET);
    // could fetch company info and json_decode it if it turns out to be needed

    if (!($ticket && ($receiveResponseXML->ticket == $ticket))) {
      watchdog('QBWC', 'QBWC::receiveResponseXML[' . $ticket . ']( ' .
        $receiveResponseXML->ticket . ', ' .
        $receiveResponseXML->strHCPResponse . ', ' .
        $receiveResponseXML->strCompanyFileName . ', ' .
        $receiveResponseXML->qbXMLCountry . ', ' . $receiveResponseXML->qbXMLMajorVers . '.' . $receiveResponseXML->qbXMLMinorVers . ' )', NULL, 'error');
      $ret->receiveResponseXMLResult = -1; // per Intuit QBWC docs
      $lastErr = 'RR:' . QBWC_INT_ERR_TICKET_MISMATCH . // ':' . $ticket .
        ':' . $receiveResponseXML->ticket;
      apc_store(QBWC_INT_LASTERR, $lastErr, QBWC_INT_TTL);
      return $ret;
    }

    // send to hook qbwc_receiveResponseXML()
    if (sizeof(module_implements(QBWC_HOOK_RECEIVERESPONSE)) > 0) {
      // Call all modules that implement the hook, and let them make changes to $variables.
      $ret->receiveResponseXMLResult = implode('',
        module_invoke_all(QBWC_HOOK_RECEIVERESPONSE,
          $ticket,
          $receiveResponseXML->response,
          $receiveResponseXML->hresult,
          $receiveResponseXML->message));

      //watchdog('QBWC', 'QBWC:receiveResponseXML(' . $receiveResponseXML->response . ', ' .
      //  $receiveResponseXML->hresult . ', ' . $receiveResponseXML->message . ')=(' . $ret->receiveResponseXMLResult . ')');
    }
    else {
      watchdog('QBWC', 'QBWC:receiveResponseXML-nohook(' . $receiveResponseXML->response . ', ' .
        $receiveResponseXML->hresult . ', ' . $receiveResponseXML->message . ')', NULL, 'error');
      $ret->receiveResponseXMLResult = 100;
    }

    $ret->receiveResponseXMLResult += 0;
    // watchdog('QBWC-dump', _var_dump2str($ret));
    return $ret;
  }


  public function connectionError($connectionError) {
    $ret = new \stdClass();
    $ticket = apc_fetch(QBWC_INT_TICKET);

    watchdog('QBWC', 'QBWC::connectionError[' .
      $ticket . ']( ' .
      $connectionError->ticket . ', ' .
      $connectionError->hresult . ', ' .
      $connectionError->message . ')', NULL, 'error');

    $ret->connectionErrorResult = 'done'; // per Intuit QBWC docs
    return $ret;
  }


  public function getLastError($getLastError) { // parameter not used
    $ret = new \stdClass();
    $ticket = apc_fetch(QBWC_INT_TICKET);

    if (!($ticket && ($getLastError->ticket == $ticket))) {
      return $ret;
    }
    $ret->getLastErrorResponse = apc_fetch(QBWC_INT_LASTERR);
    return $ret;
  }


  public function closeConnection($closeConnection) {
    $ret = new \stdClass();
    $ticket = apc_fetch(QBWC_INT_TICKET);

    if (!($ticket && ($closeConnection->ticket == $ticket))) {
      // original ticket got lost. this should get logged even if not debugging
      $errmsg = 'QBWC::closeConnection( ' . $closeConnection->ticket . ' != ' . $ticket . ' )';
      watchdog('QBWC', $errmsg, NULL, 'error');
      $ret->closeConnectionResult = 'CC: ' . $closeConnection->ticket;
      return $ret;
    }
    $ret->closeConnectionResult = 'QBWC::closeConnection( ' . $ticket . ' )';
    // watchdog('QBWC', 'closeConnection(' . $ticket . ')');
    $ticket = 0;
    apc_store(QBWC_INT_TICKET, $ticket, QBWC_INT_TTL);

    return ($ret);
  }
}


/**
 * Implementation of the interface().
 *
 */
function qbwc_interface_endpoint() {
  global $base_url;

  try {
    ini_set("soap.wsdl_cache_enabled", "0"); // turn off cache when changing wsdl file
    // watchdog('SOAP In', /*_headers_as_html_str() . '<p>' . */htmlspecialchars(file_get_contents('php://input')));
    $server = new \SoapServer($base_url . '/' . variable_get(QBWC_INT_ENDPT, QBWC_INT_ENDPT) . '.wsdl');
    $server->setClass('qbwc_interface_webconnector');
    _ob_reset();
    // xdebug_disable();
    $server->handle();
    // watchdog('SOAP Out', /* _headers_as_html_str(headers_list()) . '<p>' .*/ htmlspecialchars(ob_get_contents()));

  } catch (Exception $e) {
    watchdog('QBWC', 'NOSOAP ' . $e->getMessage(), NULL, 'error');
    // dpm($e);
  }
  exit;
}

