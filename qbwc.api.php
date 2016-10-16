<?php
/**
 * @file
 * Hooks provided by the qbwc module.
 */

/**
 * @defgroup qbwc_api_hooks QBWC_interface API Hooks
 * @{
 * Functions to send and receive Quickbooks XML requests and responses through
 * a SOAP connection to the Quickbooks Web Connector
 *
 * Quickbooks Web Connector is a program that runs natively on the machine that
 * hosts a Quickbooks application. It acts as a SOAP client. The Quickbooks Web
 * Connector gets information about the connection and server from a .QWC file.
 * QBWC Interface generates this file through its admin interface and the file
 * must then be placed on the Quickbooks host machine for the Web Connector to
 * read.
 *
 * Authentication is performed using a simple username / password combination
 * that is used ONLY for this single connection. The username and password are
 * not saved or used anywhere else. QBWC Interface allows dynamic setting of
 * multiple username, password, and endpoint combinations (on the Drupal
 * machine) through its administrative interface.
 *
 * The QBWC module is a service hosted by a Drupal installation. It acts as a
 * SOAP server. The SOAP protocol and endpoint are defined by a .WSDL file which
 * is used by SOAP client and SOAP server. qbwc generates the WSDL file as the
 * endpoint plus the extension ".wsdl" [The Quickbooks Web Connector does not
 * actually use the generated file, but rather an internal copy.]
 *
 * The Quickbooks Web Connector uses an XML-formatted configuration file with
 * the .QWC extension. There are many options and a particular syntax for this
 * file. qbwc generates a .QWC configuration file with a basic set of default
 * parameters that can be used as-is or modified as needed.
 *
 * The QBWC module establishes endpoints and then accepts a SOAP connection from
 * the Quickbooks Web Connector on each endpoint and authenticates the
 * connection using username / password combinations. Each endpoint / username /
 * password combination is associated with an internal bookname which is used to
 * route incoming and outgoing data and make sure it gets stored properly.
 * Ultimately, there might be multiple users with multiple books stored on a
 * Drupal site and the bookname will be used to distinguish each.
 *
 * After this authentication phase, the Quickbooks Web Connector sends a message
 * indicating that it is ready to accept Quickbooks XML commands / queries and
 * the Drupal site then becomes the client with the Web Connector acting as
 * intermediary to the Quickbooks application which acts as the server for this
 * QBXML exchange.
 *
 * The protocol is documented in the Quickbooks Web Connector and Quickbooks XML
 * programmer / API guides provided by Intuit
 *
 * QBWC Interface acts as the conduit for the SOAP connection and does nothing
 * with QB XML messages other than to pass them back and forth and sort them by
 * bookname.
 *
 * There are two hooks provided, which  correspond to the naming and conventions
 * of the defined Quickbooks Web Connector SOAP service:
 * - send a QB XML request from Drupal to Quickbooks
 *   - hook_qbwc_sendRequestXML()
 *
 * - receive a QB XML response from Quickbooks
 *   - hook_qbwc_receiveResponseXML()
 * @}
 */

/**
 * @addtogroup qbwc_hooks
 * @{
 */

/**
 * Get a QB XML query or command to be sent to Quickbooks Web Connector.
 *
 * This hook is used when Quickbooks Web Connector asks whether there are any
 * requests to be had. The response to this will be the command or query written
 * in QB XML which is to be sent back to Quickbooks. The arguments to this hook
 * are the bookname, an object with information arguments passed by the QB Web
 * Connector as defined in the QB SDK and the company file which was sent during
 * the authentication phase. A function using this hook should return a QBXML
 * request string to get passed to the QB Web Connector as the argument of the
 * sendRequestResponse.
 *
 **/

/*
 * Get a request (if any) to be sent to Quickbooks.
 *
 * This hook is invoked from the QBWC interface endpoint when it is received
 * through the SOAP connection. The response string must be a valid formatted QB
 * XML string to be back to Quickbooks via the sendRequestXMLResponse SOAP
 * method defined in the WSDL.
 *
 * The web connector’s invitation to the web service to send a request.
 * @param $bookname
 *   a session ID used to identify this set of exchanged SOAP messages.
 * @param $response
 *   an object containing the following Quickbooks request information.
 *   Properties of the object are:
 *     ticket -- this is like a session ID
 * 
 *     strHCPResponse -- a response string from the Quickbooks with information
 *     about the company file. Only for the first sendRequestXML call in a data
 *     exchange session will this parameter contains response data from a
 *     HostQuery, a CompanyQuery, and a PreferencesQuery request. This data is
 *     provided at the outset of a data exchange because it is normally useful
 *     for a web service to have this data. In the ensuing data exchange
 *     session, subsequent sendRequestXML calls from the web processor do not
 *     contain this data, (only an empty string is supplied) as it is assumed
 *     your web service already has it for the session.
 * 
 *     strCompanyFileName -- filename of the Quickbooks file on its native 
 *       computer
 *     qbXMLCountry -- a country code for the file / QB version
 *     qbXMLMajorVers -- number of the major version number of QB XML being used
 *     qbXMLMinorVers -- number of the minor version number of QB XML being used
 * 
 *   @return If the web service has no requests to send, specify an empty
 *   string. If you want the Web Connector to pause for an interval of time
 *   (currently 5 seconds) return the string “NoOp”, which will cause the Web
 *   Connector to call your getLastError callback: a “NoOp” returned from
 *   GetLastError will cause the QBWC to pause updates for 5 seconds before
 *   attempting to call sendRequestXML() again. Any other string will be taken
 *   as a qbXML for QuickBooks or a qbposXML request for QuickBooks POS. The Web
 *   Connector sends the qbXML or qbposXML to QuickBooks or QuickBooks POS via
 *   the request processor’s ProcessRequest method call.

 * */
function hook_qbwc_sendRequestXML($bookname, $response, SimpleXMLElement $company) {
  echo('sendRequestXML[' . $ticket . ']\nCompanyInfo(' . $company . ')\nCompanyFile(' . $strCompanyFileName .
    ')\nCountryCode(' . $qbXMLCountry . ')\nXMLVersion(' . $qbXMLMajorVers . '.' . $qbXMLMinorVers . ')\n');
  return ('<?qbxml version="5.0" ?>
<QBXML>
<QBXMLMsgsRq onError="stopOnError">
<CustomerQueryRq requestID="5001" iterator="Start">
<MaxReturned>10</MaxReturned>
<IncludeRetElement>ListID</IncludeRetElement>
</CustomerQueryRq>
</QBXMLMsgsRq>
</QBXML>'); // this example from the Quickbooks XML Programmers Guide in the SDK
}

/*
 * Get a query response from Quickbooks.
 *
 * This hook is invoked from the QBWC interface endpoint when a response is
 * received through the SOAP connection. The hook processes the information in
 * the response and returns a valid formatted QB XML response string to be sent
 * back to Quickbooks via the receiveResponseXMLResponse SOAP method defined in
 * the WSDL.
 *
 * Usage: When the web connector gets a response from QuickBooks or QuickBooks
 * POS, it sends the response to the web service through receiveResponseXML. The
 * web service should process the response and return an integer. A positive
 * integer if you want it to serve as the estimated percent complete for the
 * session, a negative integer if you want to indicate to the web connector that
 * an error has occurred.
 *
 * If the return value is positive, but less than 100 then the web connector
 * knows that the web service has additional requests to be sent to QuickBooks,
 * the connection status bar will be updated based on the percentage returned by
 * the web service and the connector will call sendRequestXML again (this time
 * leaving the strHCPResponse parameter as an empty string).
 *
 * If the return value is negative, meaning an error occurred, then the Web
 * Connector will call the web service’s getLastError method (the fourth of the
 * six required methods for your web service to implement). The getLastError
 * method returns to the error message that should be presented to the user.

 * @param $bookname
 *   an internal machine name to route data
 * @param $response
 *   object contains the qbXML response from QuickBooks or qbposXML response
 *   from QuickBooks POS with auxiliary information. Properties:
 * 
 *     ticket the session ID for this Webconnector session. Note that this is
 *     not a cookie nor HTML session ID, but rather a secure encoded session ID
 *     used by the Webconnector and Drupal
 * 
 *     response the QBXML string response from QB
 * 
 *     hresult string for error handling (see Intuit docs)
 * 
 *     message string for error handling (see Intuit docs)
 * 
 *     The hresult and message could be returned as a result of certain errors
 *     that could occur when QuickBooks or QuickBooks POS sends requests is to
 *     the QuickBooks / QuickBooks POS request processor via the ProcessRequest
 *     call. If this call to the request processor resulted in an error
 *     (exception) instead of a response, then the web connector will return the
 *     corresponding HRESULT and its text message in the hresult and message
 *     parameters. If no such error occurred, hresult and message will be empty
 *     strings.
 * 
 *   company is the original comany query string executed by Webconnector at the
 *   beginning of every session. It is stored by the QBWC module and passed
 *   with each request just for informational purposes along with the unique
 *   session ID / ticket ID associated with this exchange.
 * 
 * @return
 *   Return Value: A string representing a positive integer less than 100
 *   represents the percentage of work completed. A value of 1 means one percent
 *   complete, a value of 100 means 100 percent complete--there is no more work.
 *   A negative value means an error has occurred and the Web Connector responds
 *   to this with a getLastError call. The negative value could be used as a
 *   custom error code.
 **/
function hook_qbwc_receiveResponseXML($bookname, $response, $company) {
  echo('receiveResponse[' . $bookname . ']\nresponse(' . $response . 
    ')\ncompany(' . $company . ')\n');
  return ('100'); // 100% done
}
