<?php
/**
 * @file
 * Admin page callback for the qbwc_interface module.
 */

/**
 * Builds and returns the admin settings form.
 */
function qbwc_interface_admin_settings_form() {
  $form[QBWC_INT_USER] = array(
    '#title' => t('.QWC username'),
    '#type' => 'textfield',
    '#default_value' => variable_get(QBWC_INT_USER, drupal_random_key(16)),
    '#description' => t('Set the username for Quickbooks Web Connector.'),
    '#required' => TRUE,
  );
  $form[QBWC_INT_PASSWORD] = array(
    '#title' => t('.QWC password'),
    '#type' => 'textfield',
    '#default_value' => variable_get(QBWC_INT_PASSWORD, drupal_random_key(16)),
    '#description' => t('Set the password for Quickbooks Web Connector.'),
    '#required' => TRUE,
  );
  $form[QBWC_INT_ENDPT] = array(
    '#title' => t('Endpoint for Quickbooks WebConnector'),
    '#type' => 'textfield',
    '#default_value' => variable_get(QBWC_INT_ENDPT, drupal_random_key(8)),
    '#description' => t('Endpoint for Quickbooks Web Connector.'),
    '#required' => TRUE,
  );

  $form[QBWC_INT_FILEID] = array(
    '#title' => t('QWC FileID for this QWC file'),
    '#type' => 'textfield',
    '#default_value' => variable_get(QBWC_INT_FILEID, qbwc_interface_GUID()),
    '#description' => t('FileID for Quickbooks Web Connector see QBWC documentation.'),
    '#required' => TRUE,
  );

  $form[QBWC_INT_OWNERID] = array(
    '#title' => t('QWC OwnerID for this QWC file'),
    '#type' => 'textfield',
    '#default_value' => variable_get(QBWC_INT_OWNERID, qbwc_interface_GUID()),
    '#description' => t('OwnerID for Quickbooks Web Connector see QBWC documentation.'),
    '#required' => TRUE,
  );

  $form['#submit'][] = ('qbwc_interface_admin_settings_form_submit');

  return system_settings_form($form);
}

/**
 * Submit function for input_simple_form().
 */
function qbwc_interface_admin_settings_form_submit($form_id, &$form_state) {
  // endpoint and associated .QWC and .WSDL generation utilities reside in menu, so needs regeneration
  menu_rebuild();
}

/**
 * Deliver QWC file used to configure the Web Connector
 */
function qbwc_interface_qwc_output() {
  global $base_url;

  if(!(isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] != 'off' || stristr($base_url, 'localhost'))) {
    drupal_set_message(t('THIS WON\'T WORK!!! QBWC Interface REQUIRES secure HTTP (HTTPS) and you are running standard HTTP!!! You need to enable a secure server. Please reconfigure your server.'));
    return;
  }

  $endpoint = variable_get(QBWC_INT_ENDPT);
  $service_endpoint = $base_url . '/' . $endpoint;
  $username = variable_get(QBWC_INT_USER);
  $ownerid = variable_get(QBWC_INT_OWNERID);
  $fileid = variable_get(QBWC_INT_FILEID); // set for this endpoint and QWC connection
  $site_name = variable_get('site_name', 'Drupal@' . $base_url);
  $readonly = variable_get(QBWC_INT_READONLY, 'false'); // can make this part of the form settings if needed
  $authflags = variable_get(QBWC_INT_AUTHFLAGS, '0xf'); // see QBWC docs, compatibility flags for QB styles
  $qbtype = variable_get(QBWC_INT_QBTYPE, 'QBFS'); // can make this part of the form settings if needed
  $qwc_content = <<< QBWCXML
<QBWCXML>
  <AppName>$site_name qbwc_interface connection</AppName>
  <AppID></AppID>
  <AppURL>$service_endpoint</AppURL>
  <AppDescription>$site_name interface to QB Web Connector.</AppDescription>
  <AppSupport>$service_endpoint</AppSupport>
  <UserName>$username</UserName>
  <OwnerID>{{$ownerid}}</OwnerID>
  <FileID>{{$fileid}}</FileID>
  <IsReadOnly>$readonly</IsReadOnly>
  <QBType>$qbtype</QBType>
  <AuthFlags>$authflags</AuthFlags>
</QBWCXML>
QBWCXML;

  drupal_add_http_header('Content-Disposition', 'attachment; filename="' . $endpoint . '.qwc"', FALSE);
  qbwc_interface_print_xml_exit($qwc_content, 'application/xml; charset=utf-8');
}
