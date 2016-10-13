<?php
/**
 * @file
 * Admin page callback for the qbwc_interface module.
 */

require_once "qbwc_interface.inc";

/**
 * @file
 * QB Webconnector Interface endpoint admin pages
 */
function qbwc_interface_admin_overview() {
  $base_path = QBWC_INTERFACE_MENUROUTE;
  $destination = drupal_get_destination();
  $header = array(
    'endpoint' => t('Endpoint'),
    'qwc_username' => t('User'),
    'qwc_qbtype' => t('Type'),
    'qwc_isreadonly' => t('Readonly'),
    'qwc_authflags' => t('Authflags'),
    'operations' => t('Operations'),
  );

  $options = array();
  foreach (_qbwc_interface_endpoint_list() as $endpoint) {
    $key = $endpoint->id;
    $options[$key] = array(
      'endpoint' => check_plain($endpoint->endpoint),
      'qwc_username' => check_plain($endpoint->qwc_username),
      'qwc_qbtype' => check_plain($endpoint->qwc_qbtype),
      'qwc_isreadonly' => t($endpoint->qwc_isreadonly),
      'qwc_authflags' => check_plain($endpoint->qwc_authflags),
    );
    $operations['edit'] = array(
      'title' => t('Edit'),
      'href' => $base_path . '/' . $endpoint->id . '/edit',
      'query' => $destination,
    );
    $operations['delete'] = array(
      'title' => t('Delete'),
      'href' => $base_path . '/' . $endpoint->id . '/delete',
      'query' => $destination,
    );
    $operations[''] = array(
      'title' => t('Generate QWC'),
      'href' => $base_path . '/' . $endpoint->id . '/genqwc',
      'query' => $destination,
    );
    $options[$endpoint->id]['operations'] = array(
      'data' => array(
        '#theme' => 'links',
        '#links' => $operations,
        '#attributes' => array('class' => array('links', 'inline')),
      ),
    );
  }
  $form['add']['#markup'] = l(t('Add a new endpoint'), $base_path . '/add', array('query' => $destination));

  $form['endpoints'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $options,
    '#empty' => t('No endpoints created yet.'),
  );

  return $form;
}

/**
 * Edit form for an endpoint
 */
function qbwc_interface_endpoint_edit_form($form, &$form_state, $endpoint, $op) {
  if (!$form_state['input']) {
    if (!$endpoint) {
      $endpoint = array(
        'id' => NULL,
        'endpoint' => strtr(drupal_random_key(8), '-', '_'),
        'qwc_username' => strtr(drupal_random_key(8), '-', '_'),
        'password' => strtr(drupal_random_key(8), '-', '_'),
        'qwc_fileid' => qbwc_interface_GUID(),
        'qwc_ownerid' => qbwc_interface_GUID(),
        'qwc_authflags' => '0xf',
        'qwc_isreadonly' => 'false',
        'qwc_qbtype' => 'QBFS'
      );
    }
    else {
      if ($op == 'genqwc') {
        qbwc_interface_qwc_output($endpoint);
      }
      if ($op == 'delete') {
        db_delete('qbwc')->condition('id', $endpoint->id)->execute();
        drupal_goto(isset($_GET['destination'])
          ? $_GET['destination']
          : isset($_REQUEST['destination'])
            ? $_REQUEST['destination']
            : QBWC_INTERFACE_MENUROUTE);
        // drupal_goto( $form_state['redirect'] );
      }
      if ($op == 'import') {
        // @todo: prompt for file, upload to private, parse XML into endpoint, delete file, and reload page
      }
      $endpoint = (array) $endpoint;
    }
  }
  else {
    $endpoint = $endpoint ? (array) $endpoint : $form_state['input']['endpoint'];
  }
  $form['endpoint'] = array(
    '#tree' => TRUE,
    'id' => array(
      '#type' => 'value',
      '#value' => isset($endpoint['id']) ? $endpoint['id'] : NULL
    ),
    'bookname' => array(
      '#title' => t('Book machine name'),
      '#type' => 'textfield',
      "#description" => t('Drupal data ID to be passed w. requests: eg. Book ')
    ),
    'endpoint' => array(
      '#title' => t('Endpoint'),
      '#type' => 'textfield',
      '#default_value' => $endpoint['endpoint'],
      '#required' => TRUE,
      '#description' => t('This is an endpoint path that will receive connections from a Webconnector.')
    ),
    'qwc_username' => array(
      '#type' => 'textfield',
      '#title' => t('User name'),
      '#default_value' => $endpoint['qwc_username'],
      '#required' => TRUE,
      '#description' => t('Username for authentication (not related to QB nor Drupal users)')
    ),
    'password' => array(
      '#type' => 'textfield',
      '#title' => t('Password'),
      '#default_value' => $endpoint['password'],
      '#required' => TRUE,
      '#description' => t('Password for authentication (not related to QB nor Drupal users)')
    ),
    'qwc_ownerid' => array(
      '#type' => 'textfield',
      '#title' => t('Owner ID'),
      '#default_value' => $endpoint['qwc_ownerid'],
      '#required' => TRUE,
      '#description' => t('GUID of ownerID for this connection for QWC file')
    ),
    'qwc_fileid' => array(
      '#type' => 'textfield',
      '#title' => t('File ID'),
      '#default_value' => $endpoint['qwc_fileid'],
      '#required' => TRUE,
      '#description' => t('GUID of fileID for this connection for QWC file')
    ),
    'qwc_isreadonly' => array(
      '#type' => 'select',
      '#options' => array('true' => 'true', 'false' => 'false'),
      '#title' => t('Readonly flag'),
      '#default_value' => $endpoint['qwc_isreadonly'],
      '#required' => TRUE,
      '#description' => t('isReadonly flag for QWC file')
    ),
    'qwc_qbtype' => array(
      '#type' => 'select',
      '#title' => t('Quickbooks type'),
      '#options' => array('QBFS' => 'QBFS', 'QBPOS' => 'QBPOS'),
      '#default_value' => $endpoint['qwc_qbtype'],
      '#required' => TRUE,
      '#description' => t('QBFS for freestanding, QBPOS for point of sale')
    ),
    'qwc_authflags' => array(
      '#type' => 'textfield',
      '#title' => t('Authorization flags'),
      '#default_value' => $endpoint['qwc_authflags'],
      '#required' => TRUE,
      '#description' => t('Flags for access to QB')
    ),
  );
  $form['buttons']['update'] = array(
    '#type' => 'submit',
    '#value' => t('Save')
  );

  if (isset($endpoint['id'])) {
    $form['buttons']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete')
    );
    $form['buttons']['import'] = array(
      '#type' => 'submit',
      '#value' => t('Import QWC')
    );
  }

  return $form;
}

/**
 * Validate trigger
 */
function qbwc_interface_endpoint_edit_form_validate($form, &$form_state) {
  $endpoint = $form_state['values']['endpoint'];
  // if( endpoint exists in database and id different )
  if (db_select('qbwc', 'e')
    ->condition('e.endpoint', $endpoint['endpoint'])
    ->condition('e.id', $endpoint['id'], '<>')
    ->fields('e')
    ->execute()
    ->rowCount()
  ) {
    form_set_error('QBWC', t('Duplicate endpoints not allowed: ' . $endpoint['endpoint']));
  }
}

/**
 * Submit endpoint form
 */
function qbwc_interface_endpoint_edit_form_submit($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  $endpoint = $form_state['values']['endpoint'];
  if ($op == t('Save')) {
    if (empty($endpoint['id'])) {
      drupal_write_record('qbwc', $endpoint);
      drupal_set_message(t('New endpoint ' . $endpoint['endpoint'] . ' has been created.'));
    }
    else {
      db_update('qbwc')
        ->condition('id', $endpoint['id'])
        ->fields($endpoint)
        ->execute();
      drupal_set_message(t('Endpoint ' . $endpoint['endpoint'] . ' has been updated.'));
    }
  }
  elseif ($op == t('Delete')) {
    db_delete('qbwc')
      ->condition('id', $endpoint['id'])
      ->execute();
    drupal_set_message(t('Endpoint ' . $endpoint['endpoint'] . ' has been deleted.'));
  }
  // Synchronize triggers, actions, scheduled jobs, etc..
  menu_rebuild();
}

/**
 * Deliver QWC file used to configure the Web Connector
 */
function qbwc_interface_qwc_output($connection) {
  global $base_url;

  if (!(isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] != 'off' || stristr($base_url, 'localhost'))) {
    drupal_set_message(t('THIS WON\'T WORK!!! QBWC Interface REQUIRES secure HTTP (HTTPS) and you are running standard HTTP!!! You need to enable a secure server. Please reconfigure your server.'));
    return;
  }

  $endpoint = $connection->endpoint;
  $service_endpoint = $base_url . '/' . $endpoint;
  $site_name = variable_get('site_name', 'Drupal@' . $base_url);
  $qwc_content = <<< QBWCXML
<QBWCXML>
  <AppName>$site_name qbwc_interface connection</AppName>
  <AppID></AppID>
  <AppURL>$service_endpoint</AppURL>
  <AppDescription>$site_name interface to QB Web Connector.</AppDescription>
  <AppSupport>$base_url</AppSupport>
  <UserName>$connection->qwc_username</UserName>
  <OwnerID>{{$connection->qwc_ownerid}}</OwnerID>
  <FileID>{{$connection->qwc_fileid}}</FileID>
  <IsReadOnly>$connection->qwc_isreadonly</IsReadOnly>
  <QBType>$connection->qwc_qbtype</QBType>
  <AuthFlags>$connection->qwc_authflags</AuthFlags>
</QBWCXML>
QBWCXML;

  drupal_add_http_header('Content-Disposition', 'attachment; filename="' . $endpoint . '.qwc"', FALSE);
  qbwc_interface_print_xml_exit($qwc_content, 'application/xml; charset=utf-8');
}
