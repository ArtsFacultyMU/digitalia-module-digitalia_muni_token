<?php

/**
 * @file
 * Contains digitalia_muni.tokens.inc.
 *
 * This file provides digitalia_muni tokens.
 */

use Drupal\Core\Render\BubbleableMetadata;
use Drupal\media\Entity\Media;
use Drupal\file\Entity\File;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Datetime\DrupalDateTime;
use Drupal\Core\Utility\Token;
use Drupal\Component\Utility\Html;
use Drupal\digitalia_muni_token\DigitaliaMuniTokenInterface;
use Drupal\double_field\Plugin\Field\FieldWidget\DoubleField;

/**
 * Implements hook_token_info().
 */
function digitalia_muni_token_token_info()
{
	$type = [
		'name' => t('Digitalia Tokens'),
		'description' => t('Tokens for Digitalia.'),
	];

	$node['media_urls'] = [
		'name' => t("Media Urls"),
		'description' => t('URLs of related media files'),
	];

	$config = \Drupal::config('digitalia_muni_token.settings');
	$reference_fields = explode(";", $config->get('reference_fields'));

	foreach($reference_fields as $reference_type) {
		$node[$reference_type] = [
			'name' => $reference_type,
			'type' => "node",
			'description' => t("Reverse reference of type ") . $reference_type,
		];
	}

	
	$double_fields = explode(';', $config->get('double_fields'));
	$double_fields_name_prefix = DigitaliaMuniTokenInterface::DOUBLE_FIELDS_PREFIX;

	foreach($double_fields as $double_field) {
		$double_field_config_value = explode(':', $double_field);
		$double_field_name = $double_field_config_value[0];
		$double_field_index = isset($double_field_config_value[1]) ? $double_field_config_value[1] : '1';
		$node[$double_fields_name_prefix .$double_field_name.':'.$double_field_index] = [
			'name' => $double_fields_name_prefix.$double_field_name.':'.$double_field_index,
			'description' => t("Double field value with index "). $double_field_index,
		];
	}
	
	$triple_fields = explode(';', $config->get('triple_fields'));
	$triple_fields_name_prefix = DigitaliaMuniTokenInterface::TRIPLE_FIELDS_PREFIX;

	foreach($triple_fields as $triple_field) {
		$triple_field_config_value = explode(':', $triple_field);
		$triple_field_name = $triple_field_config_value[0];
		$triple_field_index = isset($triple_field_config_value[1]) ? $triple_field_config_value[1] : '1';
		$node[$triple_fields_name_prefix .$triple_field_name.':'.$triple_field_index] = [
			'name' => $triple_fields_name_prefix.$triple_field_name.':'.$triple_field_index,
			'description' => t("Triple field value with index "). $triple_field_index,
		];
	}

	$node["journal_url"] = ['name' => 'journal_url', 'description' => t("Journal URL"), ];
	$node["journal_name"] = ['name' => 'journal_name', 'description' => t("Journal name"), ];
	$node["journal_issn"] = ['name' => 'journal_issn', 'description' => t("Journal ISSN"), ];
	//$node['test_case:person'] = [
	//	'name' => "test_case:person",
	//	'type' => "node",
	//	'description' => t("Test case"),
	//];


	return [
		'types' => ['digitalia_muni' => $type],
		'tokens' => ['digitalia_muni' => $node],
	];
}

/**
 * Implements hook_tokens().
 */
function digitalia_muni_token_tokens($type, $tokens, array $data, array $options, BubbleableMetadata $bubbleable_metadata)
{
	$config = \Drupal::config('digitalia_muni_token.settings');
	$replacements = [];

	if ($type == 'digitalia_muni' && !empty($data['node'])) {
		foreach ($tokens as $name => $original) {
			//print "$name<br>";

			if ($name == 'media_urls') {
				$replacements[$original] = ' ' . digitalia_muni_url_to_media($data['node'], $config->get('media_delimiter'));
			}
			elseif (str_starts_with($name, DigitaliaMuniTokenInterface::DOUBLE_FIELDS_PREFIX)) {
				$replacements[$original] = ' ' . digitalia_muni_double_field($data['node'], $name, $original, $tokens, $options, $bubbleable_metadata);
			}			
			elseif (str_starts_with($name, DigitaliaMuniTokenInterface::TRIPLE_FIELDS_PREFIX)) {
				$replacements[$original] = ' ' . digitalia_muni_triple_field($data['node'], $name, $original, $tokens, $options, $bubbleable_metadata);
			}
			elseif (str_starts_with($name, DigitaliaMuniTokenInterface::JOURNAL_PREFIX)) {
				$replacements[$original] = ' ' . digitalia_muni_journal_url($data['node'], $name, $original, $tokens, $options, $bubbleable_metadata);
			}
			else {
				$replacements[$original] = ' ' . digitalia_muni_reference($data['node'], $name, $original, $tokens, $options, $bubbleable_metadata);
			}
		}
	}

	return $replacements;
}

/**
 * Fetches all nodes referenced in specified field
 *
 * @param object $node
 *   A core drupal node object.
 *
 * @param string $field
 *   Field name
 *
 * @return array(node)
 *   An array of referenced nodes.
 */
function get_referenced_entities($node, $field, $is_taxonomy = false)
{
	if ($node->hasField($field)) {
		#dpm("DEBUG: trying to list referenced entities for $field");

		$field_array = $node->get($field);
		$is_entity = true;

		$ids = [];
		foreach($field_array as $entity) {
			#dpm("DEBUG: found entity id = " . $entity->getValue()['target_id']);
			if (! isset($entity->getValue()['target_id'])) { 
				$is_entity = false; 
				#dpm("DEBUG: Target id not set"); 
			}
			else {
				array_push($ids, $entity->getValue()['target_id']); 
			}
		}

		if (! $is_entity) {
			return [];
		}

		$entities = [];
		foreach($ids as $id) {
			if (! $is_taxonomy) {
				$array = \Drupal::entityTypeManager()->getStorage('node')->loadByProperties(['nid' => $id]);	
			}
			else {
				$array = \Drupal::entityTypeManager()->getStorage('taxonomy_term')->loadByProperties(['tid' => $id]);
			}

			if ($array != null) {
				array_push($entities, array_values($array)[0]);
			}

		}

		return $entities;
	}
	return [];
}

/**
 * Make an article's journal name and URL available.
 *
 * @param object $node
 *   A core drupal node object.
 *
 * @param string $name
 *   Name of token.
 *
 * @param string $original
 *   Whole name of token.
 *
 * @param array $tokens
 *   Token array.
 *
 * @param array $options
 *   Array with options.
 *
 * @param BubbleableMetadata $bubbleable_metadata
 *   BubbleableMetadata object.
 *
 * @return string
 *   The tokenized value for the given data. Multiple entries are separated (default ';').
 */
function digitalia_muni_journal_url($node, $name, $original, $tokens, array $options, BubbleableMetadata $bubbleable_metadata)
{
	$config = \Drupal::config('digitalia_muni_token.settings');
	$token_service = \Drupal::token();
	$ref_field = "field_member_of";

	$lookfor_field = substr($name, strlen(DigitaliaMuniTokenInterface::JOURNAL_PREFIX));

	switch($lookfor_field) {

	case "url": $ref_content_type = "field_handle"; break;
	case "name": $ref_content_type = "title"; break;
	case "issn": $ref_content_type = "field_issn"; break;
	}

	$ref_type = $ref_content_type;
	$children = [];
	$values = [];
	
	if ($node->hasField($ref_field)) {
		
		#dpm("DEBUG: going to find journal's URL");
		$issues = get_referenced_entities($node, $ref_field);

		#dpm("DEBUG: found no issue = ". count($issues));
		foreach($issues as $issue) {
			#dpm("DEBUG: iterate through issues and ");
			$volumes = get_referenced_entities($issue, $ref_field);

			#dpm("DEBUG: found no volumes = ". count($volumes));
			foreach($volumes as $volume) {
				$journals = get_referenced_entities($volume, $ref_field);

				#dpm("DEBUG: found no of journals = ". count($journals));
				array_push($children, array_values($journals)[0]);
			}
		}
	}


	foreach($children as $child) {
		#dpm("DEBUG: iterate through journals, type = " . $child->getEntityTypeId());
		if ($child->hasField($ref_type)) { 
			#dpm("DEBUG: has field $ref_type"); 
			//$value = $child->get($ref_type)->getString();
			//$value = $child->toUrl()->toString();
			if ($ref_type == "field_handle") {
				$options = ['absolute' => TRUE];
				$value = \Drupal\Core\Url::fromRoute('entity.node.canonical', ['node' => $child->id()], $options)->toString();
				array_push($values, $value);
				#dpm("DEBUG: url values get: " . $value);
			}
			else {
				$field_values = $child->get($ref_type);
				foreach($field_values as $field_value) {
					#dpm("DEBUG: $name value = " . $field_value->getString());
					array_push($values, $field_value->getString());
				}
			}
		}
	}

	#dpm("DEBUG: end -----------------------------");
	#dpm("DEBUG: string return = ". implode($config->get('reference_delimiter'), array_unique($values)));

	return implode($config->get('reference_delimiter'), array_unique($values));

}


/**
 * Make double fields split values available.
 *
 * @param object $node
 *   A core drupal node object.
 *
 * @param string $name
 *   Name of token.
 *
 * @param string $original
 *   Whole name of token.
 *
 * @param array $tokens
 *   Token array.
 *
 * @param array $options
 *   Array with options.
 *
 * @param BubbleableMetadata $bubbleable_metadata
 *   BubbleableMetadata object.
 *
 * @return string
 *   The tokenized value for the given data. Multiple entries are separated (default ';').
 */
function digitalia_muni_double_field($node, $name, $original, $tokens, array $options, BubbleableMetadata $bubbleable_metadata)
{
	$config = \Drupal::config('digitalia_muni_token.settings');
	$double_field_name = substr(strtok($name, DigitaliaMuniTokenInterface::DOUBLE_FIELDS_INDEX_DELIMITER), 
		strlen(DigitaliaMuniTokenInterface::DOUBLE_FIELDS_PREFIX));
	$double_field_index = substr($name, 
		strrpos($name, DigitaliaMuniTokenInterface::DOUBLE_FIELDS_INDEX_DELIMITER) + 1); 

	#dpm("DEBUG: processing double field $double_field_name");

	$values = array();

	if ($node->hasField($double_field_name)) {
		$double_fields_list = $node->get($double_field_name);

		#dpm("DEBUG: processing double field $double_field_name");

		if ($double_fields_list != null) {
			$dfs = $double_fields_list->getIterator();

			foreach($dfs as $field) {
				$values[] = $double_field_index != 1 ? $field->second : $field->first;
				//#dpm("DEBUG1: ". $field->first);
				//#dpm("DEBUG2: ". $field->second);
				//#dpm("DEBUG_index: ". $double_field_index);
			}

		}
	}

	if (count($values) > 0) {
		return implode($config->get('reference_delimiter'), array_unique($values));
	}
	else {
		return null;
	}	
	//return $double_field_name . " + " . $double_field_index;
	//return $values;

}

/**
 * Make triple fields split values available.
 *
 * @param object $node
 *   A core drupal node object.
 *
 * @param string $name
 *   Name of token.
 *
 * @param string $original
 *   Whole name of token.
 *
 * @param array $tokens
 *   Token array.
 *
 * @param array $options
 *   Array with options.
 *
 * @param BubbleableMetadata $bubbleable_metadata
 *   BubbleableMetadata object.
 *
 * @return string
 *   The tokenized value for the given data. Multiple entries are separated (default ';').
 */
function digitalia_muni_triple_field($node, $name, $original, $tokens, array $options, BubbleableMetadata $bubbleable_metadata)
{
	$config = \Drupal::config('digitalia_muni_token.settings');
	$triple_field_name = substr(strtok($name, DigitaliaMuniTokenInterface::TRIPLE_FIELDS_INDEX_DELIMITER), 
		strlen(DigitaliaMuniTokenInterface::TRIPLE_FIELDS_PREFIX));
	$triple_field_index = substr($name, 
		strrpos($name, DigitaliaMuniTokenInterface::TRIPLE_FIELDS_INDEX_DELIMITER) + 1); 


	$values = array();

	#dpm("DEBUG: triple field = " . $triple_field_name);

	if ($node->hasField($triple_field_name)) {
		$triple_fields_list = $node->get($triple_field_name);


		#dpm("DEBUG: " . count($triple_fields_list));

		if ($triple_fields_list != null) {


			$dfs = $triple_fields_list->getIterator();

			foreach($dfs as $field) {
				$values[] = $field->getValue()[$triple_field_index];
				#dpm("DEBUG: triple_field_value ". $field->getValue()[$triple_field_index]);
				#dpm("DEBUG: index = ". $triple_field_index);
			}

		}
	}

	if (count($values) > 0) {
		return implode($config->get('reference_delimiter'), array_unique($values));
	}
	else {
		return null;
	}	
	//return $triple_field_name . " + " . $triple_field_index;
	//return $values;

}

/**
 * Make reverse/forward tokens available. Merges forward and reverse references, if the needs arise, should be split.
 *
 * @param object $node
 *   A core drupal node object.
 *
 * @param string $name
 *   Name of token.
 *
 * @param string $original
 *   Whole name of token.
 *
 * @param array $tokens
 *   Token array.
 *
 * @param array $options
 *   Array with options.
 *
 * @param BubbleableMetadata $bubbleable_metadata
 *   BubbleableMetadata object.
 *
 * @return string
 *   The tokenized value for the given data. Multiple entries are separated (default ';').
 */
function digitalia_muni_reference($node, $name, $original, $tokens, array $options, BubbleableMetadata $bubbleable_metadata)
{
	$config = \Drupal::config('digitalia_muni_token.settings');
	$token_service = \Drupal::token();
	$ref_fields_config = explode(";", $config->get('reference_fields'));
	$ref_type = explode(":", $name)[0];
	$ref_content_type = explode(":", $name)[1];
	$ref_ref_content_type = isset(explode(":", $name)[2]) ? explode(":", $name)[2] : '';
	$token_prefix = $ref_type . ":" . $ref_content_type;
	$children = [];
	$is_taxonomy = false;

	#dpm("DEBUG: processing $ref_type");
	// check if it is a taxonomy and if so remove taxonomy prefix and set is_taxonomy flag 
	if (str_starts_with($ref_type, DigitaliaMuniTokenInterface::TAXONOMY_FIELDS_PREFIX)) {
		#dpm("DEBUG: $ref_type is taxonomy");
		$ref_type = substr($ref_type, strlen(DigitaliaMuniTokenInterface::TAXONOMY_FIELDS_PREFIX));
		$taxonomy_field = $ref_ref_content_type;
		$is_taxonomy = true;
	}

	// allow only reference fields of content types from config
	if (!is_numeric(array_search($token_prefix, $ref_fields_config))) {
		\Drupal::logger('digitalia_muni_token')->warning("Reference type '$ref_type' not enabled.");
		#dpm("DEBUG: $token_prefix not enabled");
		return "";
	}
	else {
		#dpm("DEBUG: $token_prefix is enabled");
	}

	#dpm("DEBUG: + process $ref_type:$ref_content_type");
	// separate forward and reverse references
	// if node has field named ref_type, process it directly, otherwise it is linked vice versa
	if ($node->hasField($ref_type)) {
		
		#dpm("DEBUG: node has field, check entities in reftype = $ref_type");
		$references = get_referenced_entities($node, $ref_type, $is_taxonomy);

		#dpm("DEBUG: found no links = ". count($references));
		// depth 2 - if there's second level linked entity
		foreach($references as $reference) {
			#dpm("DEBUG: iterate through references and search for $ref_content_type");
			$ref_entities = get_referenced_entities($reference, $ref_content_type, $is_taxonomy);

			#dpm("DEBUG: found $ref_content_type no of links  " . count($ref_entities));
			if (empty($ref_entities[0])) {
				#dpm("DEBUG: $ref_content_type has no links - skip");
				break;
			}


			array_push($children, array_values($ref_entities)[0]);
		}

		// depth 1
		if (empty($children[0])) {
			$children = $references;
			#dpm("DEBUG: depth 2 not found, stay in depth 1");
		}
		else {
			#dpm("DEBUG: childer set to depth 2");
		}
	} else {
		#dpm("DEBUG: node has no direct field $ref_type, get $ref_content_type entities that references this node");
		#dpm("DEBUG: trying to find entity '$ref_type' => " . $node->id() . " and 'type' => $ref_content_type");  
		$children = \Drupal::entityTypeManager()->getStorage('node')->loadByProperties([$ref_type => $node->id(), 'type' => $ref_content_type]);
	}

	#dpm("DEBUG: we have no of entities of '$ref_type:$ref_content_type' to get values from = " . count($children));

	$values = [];
	// generate tokens
	#dpm("DEBUG: there are no of children = " . count($children));
	foreach($children as $child) {
		#dpm("DEBUG: child entity type = " . $child->getEntityTypeId());
		#dpm("DEBUG: child ref_type:ref_content_type = " . $ref_type . ":" . $ref_content_type);
		if ($child->getEntityTypeId() == 'taxonomy_term') {
			/*
			foreach($child->getFields() as $k => $f) {
				#dpm("DEBUG: child value field = " . $f . " - " . $k);
			}
			 */

			if (strlen(trim($taxonomy_field)) > 0) {
			       if ($child->hasField($taxonomy_field)) {
				       foreach($child->get($taxonomy_field) as $field_ref_list) {
					       foreach($field_ref_list->getValue() as $value) {
			       		#dpm("DEBUG: fiel:d val '$taxonomy_field' = ". $value);	
					array_push($values, $value);
					       }
				       }
			       }
			}
			else {
				#dpm("DEBUG: no txonomy field specified, get taxonomy name");	
				#dpm("DEBUG: taxonomy child value name = " . $child->getName());
			        #dpm("DEBUG: child value name translation (en) = " . $child->getTranslation('en')->getName());	
				array_push($values, $child->getName());
			}
		}
		else {
			#dpm("DEBUG: reference type is node, so get it via token service");
			// filter out reference type prefix
			if ($child_entity_tokens = $token_service->findWithPrefix($tokens, $ref_type)) {
				#dpm("DEBUG: trying to find tokens for $ref_type and entity label = " . $child->id());
				// filter out content type/field name prefix
				$added_replacements = $token_service->generate($child->getEntityTypeId(), $token_service->findWithPrefix($child_entity_tokens, $ref_content_type), [$child->getEntityTypeId() => $child], $options, $bubbleable_metadata);
				if (empty($added_replacements)) {
					continue;
				}
				#dpm("DEBUG: token values get: " . $added_replacements[$original]);
				array_push($values, $added_replacements[$original]);
			}
		}
	}

	#dpm("DEBUG: end -----------------------------");
	#dpm("DEBUG: string return = ". implode($config->get('reference_delimiter'), array_unique($values)));

	return implode($config->get('reference_delimiter'), array_unique($values));
}

/**
 * Return machine name of field containing data
 *
 * @param string $bundle
 *   Name of bundle
 *
 * @return string
 *   String with name of field containing data
 *
 * @throws
 *   Throws exeption, when lookup fails
 */
function digitalia_muni_get_field_name($bundle)
{
	$config = \Drupal::config('digitalia_muni_token.settings');
	$mappings = explode(";", $config->get('media_fields'));

	#dpm("DEBUG: processing '$bundle'");
	foreach($mappings as $map) {
		#dpm("DEBUG: checking map = " . explode(":", $map)[0]);
		if (explode(":", $map)[0] == $bundle) {
			return explode(":", $map)[1];
		}
	}

	throw new Exception("Data field lookup failed for bundle '" . $bundle . "'.");
}

/**
 * Gets published media files URLs of a given node.
 *
 * @param object $node
 *   A core drupal node object.
 *
 * @param string $delimiter
 *   A delimiter character of the result
 *
 * @return string
 *   The tokenized value for the given data. Multiple entries are separated (default ';')
 */
function digitalia_muni_url_to_media($node, $delimiter = ';')
{
	$media_array = array();
	$islandora_utils = \Drupal::service('islandora.utils');
	$media_list = $islandora_utils->getMedia($node);
	#dpm("DEBUG: media no " . count($media_list));

	// finds existing published media and lists their download URLs
	foreach($media_list as $element) {
		if (!is_object($element) || is_null($element->id())) {
			continue;
		}

		$media = Media::load($element->id());

		#dpm("DEBUG: media status = " . $media->get('status')->value);
		// skip if not published
		if ($media->get('status')->value != 1) {
			continue;
		}

		// get file field value, catch possible lookup exception
		try {
			$fid = $media->get(digitalia_muni_get_field_name($media->bundle()))->getValue();
		} catch(Exception $e) {
			\Drupal::logger('digitalia_muni_token')->error($e->getMessage());
			#dpm("DEBUG: not configured " . $e->getMessage());
			continue;
		}

		#dpm("DEBUG: found fids = ". count($fid));

		$fid_value = (is_array($fid) && array_key_exists(0, $fid) &&
			     array_key_exists('target_id', $fid[0])) ?
			     $fid[0]['target_id'] : NULL;

		#dpm("DEBUG: fid value = ". $fid_value);
		if (is_null($fid_value)) {
			continue;
		}

		$file = File::load($fid_value);
		if ($file) {
			#dpm("DEBUG: file loaded, URL: ". $islandora_utils->getDownloadUrl($file));
			array_push($media_array, $islandora_utils->getDownloadUrl($file));
		}
	}

	return implode($delimiter, $media_array);
}
