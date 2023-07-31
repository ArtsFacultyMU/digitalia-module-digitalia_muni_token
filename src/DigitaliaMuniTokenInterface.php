<?php

namespace Drupal\digitalia_muni_token;

interface DigitaliaMuniTokenInterface {
	/**
	 * Prefix for double fields.
	 */
	const DOUBLE_FIELDS_PREFIX = 'double__';
	
	/**
	 * Prefix for triple fields.
	 */
	const TRIPLE_FIELDS_PREFIX = 'triple__';

	/**
	 * Index delimiter for double fields.
	 */
	const DOUBLE_FIELDS_INDEX_DELIMITER = ':';
	
	/**
	 * Index delimiter for triple fields.
	 */
	const TRIPLE_FIELDS_INDEX_DELIMITER = ':';

	/**
	 * Index delimiter for triple fields.
	 */
	const TAXONOMY_FIELDS_PREFIX = 'taxonomy__';

	/**
	 * Index delimiter for journal processing.
	 */
	const JOURNAL_PREFIX = 'journal_';


}
