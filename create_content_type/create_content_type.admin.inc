<?php

function product_uploader_details($file_obj, &$context) {
  $fp = file($file_obj->uri);
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 1;
    $context['sandbox']['max'] = count($fp) - 1;
    $context['results']['success'] = 0;
    $context['results']['error'] = 0;
  }
  $file = new SplFileObject($file_obj->uri);
  $header = $file->current(); //Always gets the header.
  $progress = $context['sandbox']['progress'];
  $max = $context['sandbox']['max'];
  if ($max == 0) {
    drupal_set_message(t('Please enter data in sheet'), 'error');
  }
  for ($i = 0; ($i < 100) && (($i + $progress) <= $max); $i++) {
    $file->seek($i + $progress);
    $data = str_getcsv($file->current());
    $head = explode(',', $header);
    $error = FALSE;
    foreach ($data as $key => $value) {
      $data[$key] = trim($value);
      if ((is_null($data[$key]) || empty($data[$key]))) {
        $context['results']['error'] ++;
        $error_string = $head[$key] . ' is empty at row ' . ($i + $progress);
        $context['results']['error_array'][$i + $progress] .= $error_string;
        $context['sandbox']['progress'] = $i + $progress;
        continue 2;
      }
    }
    $product_title = trim($data[0]);
    $description = trim($data[1]);
    $selling_price = trim($data[2]);
    $offer_price = trim($data[3]);
    $product_image_name = trim($data[4]);
    $sku = trim($data[5]);
    $product_tag = explode(',', $data[6]);
    $product_brand = trim($data[7]);
    $flag = 0;
    $node = new stdClass();  // Create a new node object
    $node->type = 'product';  // Content type
    $node->language = LANGUAGE_NONE;  // Or e.g. 'en' if locale is enabled
    node_object_prepare($node);  //Set some default values
    $node->title = $product_title;
    $node->intelli_ct_type_description[LANGUAGE_NONE][0]['value'] = $description;
    $node->intelli_ct_type_sell_price[LANGUAGE_NONE][0]['value'] = $selling_price;
    $node->intelli_ct_type_offer_price[LANGUAGE_NONE][0]['value'] = $offer_price;
    // Create image File object and associate with Image field.
    $filename = $product_image_name;

    $filename = match_data($filename, 'sites/default/files/product_images/');
    //dpm($filename, 'filename');
    if (!empty($filename)) {
      $filepath = drupal_realpath($filename);
      $flag = 1;
      //dpm('iff');
    }
    else {
      $filename = match_data($product_image_name, 'sites/default/files/');
      //dpm($filename,'filename else');
      if (!empty($filename)) {
        $filepath = drupal_realpath($filename);
        $flag = 2;
        //dpm('else');
      }
    }
    if ($flag == 1 || $flag == 2) {
      $b = strripos($filename, '/');
      $c = substr($filename, $b + 1);
      $fid = _check_image_exist($c);
      //dpm($fid,'fid');
      if (!empty($fid)) {
        if($flag == 1){
          file_unmanaged_move($filepath, 'public://', FILE_EXISTS_REPLACE);
        }
        $img_file = file_load($fid[0]);
        //dpm($file, 'file_if');
      }
      else {
        $img_file = (object) array(
              'uid' => 1,
              'uri' => $filepath,
              'filemime' => file_get_mimetype($filepath),
              'status' => 1,
              'filename' => $c,
        );
        $img_file = file_move($img_file, 'public://');
      }
//Clear all image styles of the product so they will be rebuilt
      image_path_flush('public://' . $c);
    }
    //$file = file_load(3);
    $node->intelli_ct_type_product_image[LANGUAGE_NONE][0] = (array) $img_file;
    $node->intelli_ct_type_product_sku['und'][0]['value'] = $sku;
    $tid_result = _check_tid_exist($product_tag);
    $node->intelli_ct_type_product_tag[$node->language] = $tid_result;
    $node->intelli_ct_type_product_brand[LANGUAGE_NONE][0]['value'] = $product_brand;
    node_save($node);
    $context['results']['success'] ++;
    $context['sandbox']['progress'] = $i + $progress;
  }
  $rows_remain = $context['sandbox']['max'] - $context['sandbox']['progress'];
  $context['message'] = t($rows_remain . ' Remaining.');
  if ($context['sandbox']['progress'] < $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
  else {
    $context['finished'] = 1;
  }
  $context['results']['fid'] = $file_obj->fid;
}

function product_uploader_finish($success, $results, $operations) {
  if ($success) {
    drupal_set_message(t('Records updated successfully'));
  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array('%error_operation' => $error_operation[0], '@arguments' => print_r($error_operation[1], TRUE)));
    drupal_set_message($message, 'error');
  }
}

function _check_image_exist($file_name) {
  $query = db_select('file_managed', 'fm');
  $query->fields('fm', array('fid'));
  $query->condition('fm.filename', $file_name);
  return $query->execute()->fetchCol();
}

function match_data($file_name, $dirname) {
  $flag = false;
  foreach (glob($dirname . "*.jpg") as $filename) {
    //dpm('foreech');
    $b = strripos($filename, '/');
    $c = substr($filename, $b + 1);
    $a = strstr($c, ".jpg", true);
    //dpm($filename,'filename_match');
    //dpm($a,'a');
    //dpm($file_name,'file_name');
    if ($a == $file_name) {
      //dpm('match');
      return $filename;
    }
    else {
    //dpm('not match');  
    }
  }
  return $flag;
}

function _check_tid_exist($product_tag) {
  $return = array();
  foreach ($product_tag as $key => $value) {
    $tid_obj = taxonomy_get_term_by_name($value, 'tags');
    //dpm($tid_obj,'obj');
    if (!empty($tid_obj)) {
      $tid_array = array('tid' => array_keys($tid_obj));
    }
    else {
      $vid = taxonomy_vocabulary_machine_name_load('tags')->vid;
      $newterm = new stdClass();
      $newterm->name = $value;
      $newterm->vid = $vid;
      taxonomy_term_save($newterm);
      $tid_array = array('tid' => $newterm->tid);
    }
    $return[] = $tid_array;
  }
  return $return;
}
