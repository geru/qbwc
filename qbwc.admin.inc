<?php
/**
 * @file
 * Admin page callback for the qbwc module.
 */

require_once "qbwc.inc";

/**
 * @file
 * QB Webconnector Interface connection admin pages
 */
function qbwc_admin_overview() {
  $base_path = QBWC_MENUROUTE . '/qbwc';
  $destination = drupal_get_destination();
  $header = array(
    'bookid' => t('Book'),
    'endpoint' => t('Endpoint'),
    'qwc_username' => t('User'),
    'qwc_qbtype' => t('Type'),
    'qwc_isreadonly' => t('Readonly'),
    'qwc_authflags' => t('Authflags'),
    'qbxml_version' => t('QBXML version'),
    'operations' => t('Operations'),
  );

  $options = array();
  foreach (_qbwc_connection_list() as $connection) {
    $key = $connection->id;
    $options[$key] = array(
      'bookid' => $connection->bookid,
      'endpoint' => check_plain($connection->endpoint),
      'qwc_username' => check_plain($connection->qwc_username),
      'qwc_qbtype' => check_plain($connection->qwc_qbtype),
      'qwc_isreadonly' => t($connection->qwc_isreadonly),
      'qwc_authflags' => check_plain($connection->qwc_authflags),
      'qbxml_version' => check_plain($connection->qbxml_version)
    );
    $operations['edit'] = array(
      'title' => t('Edit'),
      'href' => $base_path . '/' . $connection->id . '/edit',
      'query' => $destination,
    );
    $operations['delete'] = array(
      'title' => t('Delete'),
      'href' => $base_path . '/' . $connection->id . '/delete',
      'query' => $destination,
    );
    $operations[''] = array(
      'title' => t('Generate QWC'),
      'href' => $base_path . '/' . $connection->id . '/genqwc',
      'query' => $destination,
    );
    $options[$connection->id]['operations'] = array(
      'data' => array(
        '#theme' => 'links',
        '#links' => $operations,
        '#attributes' => array('class' => array('links', 'inline')),
      ),
    );
  }
  $form['add']['#markup'] = l(t('Add a new connection'), $base_path . '/add', array('query' => $destination));

  $form['connections'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $options,
    '#empty' => t('No connections created yet.'),
  );

  return $form;
}

/**
 * Edit form for an connection
 */
function qbwc_connection_edit_form($form, &$form_state, $connection, $op) {
  if (!$form_state['input']) {
    if (!$connection) {
      $connection = array(
        'id' => NULL,
        'bookid' => 0,
        'endpoint' => strtr(drupal_random_key(8), '-', '_'),
        'qwc_username' => strtr(drupal_random_key(8), '-', '_'),
        'password' => strtr(drupal_random_key(8), '-', '_'),
        'qwc_fileid' => qbwc_GUID(),
        'qwc_ownerid' => qbwc_GUID(),
        'qwc_authflags' => '0xf',
        'qwc_isreadonly' => 'false',
        'qwc_qbtype' => 'QBFS',
        'qbxml_version' => '13.0',
      );
    }
    else {
      if ($op == 'genqwc') {
        qbwc_qwc_output($connection);
      }
      if ($op == 'delete') {
        db_delete('qbwc')->condition('id', $connection->id)->execute();
        drupal_goto(isset($_GET['destination'])
          ? $_GET['destination']
          : isset($_REQUEST['destination'])
            ? $_REQUEST['destination']
            : QBWC_MENUROUTE);
      }
      if ($op == 'import') {
        // @todo: prompt for file, upload to private, parse XML into connection, delete file, and reload page
      }
      $connection = (array) $connection;
    }
  }
  else {
    $connection = $connection ? (array) $connection : $form_state['input']['connection'];
  }
  $form['connection'] = array(
    '#tree' => TRUE,
    'id' => array(
      '#type' => 'value',
      '#value' => isset($connection['id']) ? $connection['id'] : NULL
    ),
    'bookid' => array(
      '#title' => t('Book machine id'),
      '#type' => 'textfield',
      '#description' => t('Drupal book / dataset ID to be passed w. requests: eg. 1 '),
      '#default_value' => $connection['bookid'] ? $connection['bookid'] : 0,
      '#element_validate' => array('element_validate_integer_positive'),
    ),
    'endpoint' => array(
      '#title' => t('Endpoint'),
      '#type' => 'textfield',
      '#default_value' => $connection['endpoint'],
      '#required' => TRUE,
      '#description' => t('This is an endpoint path that will receive connections from a Webconnector.')
    ),
    'qwc_username' => array(
      '#type' => 'textfield',
      '#title' => t('User name'),
      '#default_value' => $connection['qwc_username'],
      '#required' => TRUE,
      '#description' => t('Username for authentication (not related to QB nor Drupal users)')
    ),
    'password' => array(
      '#type' => 'textfield',
      '#title' => t('Password'),
      '#default_value' => $connection['password'],
      '#required' => TRUE,
      '#description' => t('Password for authentication (not related to QB nor Drupal users)')
    ),
    'qwc_ownerid' => array(
      '#type' => 'textfield',
      '#title' => t('Owner ID'),
      '#default_value' => $connection['qwc_ownerid'],
      '#required' => TRUE,
      '#description' => t('GUID of ownerID for this connection for QWC file')
    ),
    'qwc_fileid' => array(
      '#type' => 'textfield',
      '#title' => t('File ID'),
      '#default_value' => $connection['qwc_fileid'],
      '#required' => TRUE,
      '#description' => t('GUID of fileID for this connection for QWC file')
    ),
    'qwc_isreadonly' => array(
      '#type' => 'select',
      '#options' => array('true' => 'true', 'false' => 'false'),
      '#title' => t('Readonly flag'),
      '#default_value' => $connection['qwc_isreadonly'],
      '#required' => TRUE,
      '#description' => t('isReadonly flag for QWC file')
    ),
    'qwc_qbtype' => array(
      '#type' => 'select',
      '#title' => t('Quickbooks type'),
      '#options' => array('QBFS' => 'QBFS', 'QBPOS' => 'QBPOS'),
      '#default_value' => $connection['qwc_qbtype'],
      '#required' => TRUE,
      '#description' => t('QBFS for freestanding, QBPOS for point of sale')
    ),
    'qwc_authflags' => array(
      '#type' => 'textfield',
      '#title' => t('Authorization flags'),
      '#default_value' => $connection['qwc_authflags'],
      '#required' => TRUE,
      '#description' => t('Flags for access to QB')
    ),
    'qbxml_version' => array(
      '#type' => 'textfield',
      '#title' => t('QBXML version'),
      '#default_value' => $connection['qbxml_version'],
      '#required' => TRUE,
      '#description' => t('QBXML version number passed in header')
    ),
  );
  $form['buttons']['update'] = array(
    '#type' => 'submit',
    '#value' => t('Save')
  );

  if (isset($connection['id'])) {
    $form['buttons']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete')
    );
    // @todo: import info from QWC file: $form['buttons']['import'] = array('#type' => 'submit','#value' => t('Import QWC'));
  }

  return $form;
}

/**
 * Validate trigger
 */
function qbwc_connection_edit_form_validate($form, &$form_state) {
  $connection = $form_state['values']['connection'];
  // if( connection exists in database and id different )
  if (db_select('qbwc', 'e')
    ->condition('e.endpoint', $connection['endpoint'])
    ->condition('e.id', $connection['id'], '<>')
    ->fields('e')
    ->execute()
    ->rowCount()
  ) {
    form_set_error('QBWC', t('Duplicate connections not allowed: ' . $connection['endpoint']));
  }
}

/**
 * Submit connection form
 */
function qbwc_connection_edit_form_submit($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  $connection = $form_state['values']['connection'];
  if ($op == t('Save')) {
    if (empty($connection['id'])) {
      drupal_write_record('qbwc', $connection);
      drupal_set_message(t('New connection ' . $connection['endpoint'] . ' has been created.'));
    }
    else {
      db_update('qbwc')
        ->condition('id', $connection['id'])
        ->fields($connection)
        ->execute();
      drupal_set_message(t('Connection ' . $connection['endpoint'] . ' has been updated.'));
    }
  }
  elseif ($op == t('Delete')) {
    db_delete('qbwc')
      ->condition('id', $connection['id'])
      ->execute();
    drupal_set_message(t('connection ' . $connection['endpoint'] . ' has been deleted.'));
  }
  // Synchronize triggers, actions, scheduled jobs, etc..
  menu_rebuild();
}

/**
 * Deliver QWC file used to configure the Web Connector
 */
function qbwc_qwc_output($connection) {
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
  <AppName>$site_name qbwc connection</AppName>
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
  // todo: write all the optional properties if they are set
  drupal_add_http_header('Content-Disposition', 'attachment; filename="' . $endpoint . '.qwc"', FALSE);
  qbwc_print_xml_exit($qwc_content, 'application/xml; charset=utf-8');
}
