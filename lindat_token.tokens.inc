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
use Drupal\Core\Datetime\DrupalDateTime;
use Drupal\Core\Utility\Token;
use Drupal\Component\Utility\Html;

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
			'description' => t("Reverese reference of type ") . $reference_type,
		];
	}


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
			switch ($name) {
			case 'media_urls':
				$replacements[$original] = ' ' . digitalia_muni_url_to_media($data['node'], $config->get('delimiter'));
				break;
			// reverse references
			default:
				$replacements[$original] = ' ' . digitalia_muni_reverse_reference($data['node'], $name, $original, $tokens, $options, $bubbleable_metadata);
				break;
			}
		}
	}

	return $replacements;
}

/**
 * Gets published media files URLs of a given node.
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
function digitalia_muni_reverse_reference($node, $name, $original, $tokens, array $options, BubbleableMetadata $bubbleable_metadata)
{
	$config = \Drupal::config('digitalia_muni_token.settings');
	$token_service = \Drupal::token();
	$reference_fields = explode(";", $config->get('reference_fields'));
	$reference_type = explode(":", $name)[0];
	$size = sizeof(explode(":", $name));

	// allow only reference fields from config
	if (!is_numeric(array_search($reference_type, $reference_fields))) {
		return "";
	}

	$children = \Drupal::entityTypeManager()->getStorage('node')->loadByProperties([$reference_type => $node->id()]);
	$values = [];

	foreach($children as $child) {
		if (($child_entity_tokens = $token_service->findWithPrefix($tokens, $reference_type)) && $size > 1) {
			$added_replacements = $token_service->generate($child->getEntityTypeId(), $child_entity_tokens, [$child->getEntityTypeId() => $child], $options, $bubbleable_metadata);
			if (empty($added_replacements)) {
				continue;
			}
			array_push($values, $added_replacements[$original]);
		} else {
			array_push($values, $child->label());
		}
	}
	return implode($config->get('delimiter'), $values);
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

	foreach($mappings as $map) {
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

	// finds existing published media and lists their download URLs
	foreach($media_list as $element) {
		if (!is_object($element) || is_null($element->id())) {
			continue;
		}

		$media = Media::load($element->id());

		// skip if not published
		if ($media->get('status')->value != 1) {
			continue;
		}

		// get file field value, catch possible lookup exception
		try {
			$fid = $media->get(digitalia_muni_get_field_name($media->bundle()))->getValue();
		} catch(Exception $e) {
			\Drupal::logger('digitalia_muni_token')->error($e->getMessage());
			continue;
		}

		$fid_value = (is_array($fid) && array_key_exists(0, $fid) &&
			     array_key_exists('target_id', $fid[0])) ?
			     $fid[0]['target_id'] : NULL;

		if (is_null($fid_value)) {
			continue;
		}

		$file = File::load($fid_value);
		if ($file) {
			array_push($media_array, $islandora_utils->getDownloadUrl($file));
		}
	}

	return implode($delimiter, $media_array);
}
