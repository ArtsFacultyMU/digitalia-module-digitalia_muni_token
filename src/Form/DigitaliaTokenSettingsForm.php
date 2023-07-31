<?php

namespace Drupal\digitalia_muni_token\Form;

use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Form\FormStateInterface;

class DigitaliaTokenSettingsForm extends ConfigFormBase
{
	/**
	 * {@inheritdoc}
	 */
	public function getFormId()
	{
		return 'digitalia_muni_token_form';
	}

	/**
	 * {@inheritdoc}
	 */
	public function buildForm(array $form, FormStateInterface $form_state)
	{
		$form = parent::buildForm($form, $form_state);
		$config = $this->config('digitalia_muni_token.settings');
		print $config->get('digitalia_muni_token.media_delimiter');
		print '<br/>';
		print $config->get('digitalia_muni_token.media_fields');
		print '<br/>';

		$form['media_delimiter'] = [
			'#type' => 'textfield',
			'#title' => $this->t('Media fields delimiter'),
			'#default_value' => $config->get('media_delimiter'),
			'#description' => $this->t('Delimiter used to separate multiple media URLs.'),
		];

		$form['media_fields'] = [
			'#type' => 'textarea',
			'#size' => 60,
			'#title' => $this->t('Media field mapping'),
			'#default_value' => $config->get('media_fields'),
			'#description' => $this->t('Mapping of \'media name\':\'field with media\', separated by \';\'.'),
		];

		$form['reference_delimiter'] = [
			'#type' => 'textfield',
			'#title' => $this->t('Reference fields delimiter'),
			'#default_value' => $config->get('reference_delimiter'),
			'#description' => $this->t('Delimiter used to separate multiple descendent fields.'),
		];

		$form['reference_fields'] = [
			'#type' => 'textarea',
			'#size' => 60,
			'#title' => $this->t('Allowed reference fields'),
			'#default_value' => $config->get('reference_fields'),
			'#description' => $this->t('Allowed reference fields, separated by \';\' an in format of "field:entity". The referenced fields can be of different types - depends whether the node is referenced by an entity or reference an entity via field. In first case the "entity" is the name of entity which reference the node and "field" is the name of the name of the field in the entity that references the node . In the second case the field is an entity reference field name and there are two options for "entity": a) we want to get direct field value from entity so the "entity" can be custom value, b) we want to go deeper and wants a link to entity of the "entity" and the "entity" must be entity name.'),
		];

		$form['double_fields'] = [
			'#type' => 'textarea',
			'#size' => 60,
			'#title' => $this->t('Allowed double fields'),
			'#default_value' => $config->get('double_fields'),
			'#description' => $this->t('Allowed double fields, separated by \';\', each in format <i>field</i>:[1|2].'),
		];
		
		$form['triple_fields'] = [
			'#type' => 'textarea',
			'#size' => 60,
			'#title' => $this->t('Allowed triple fields'),
			'#default_value' => $config->get('triple_fields'),
			'#description' => $this->t('Allowed triple fields, separated by \';\', each in format <i>field</i>:[1|2|3].'),
		];

		return $form;
	}

	/**
	 * {@inheritdoc}
	 */
	public function validateForm(array &$form, FormStateInterface $form_state){}


	/**
	 * {@inheritdoc}
	 */
	public function submitForm(array &$form, FormStateInterface $form_state)
	{
		$config = $this->config('digitalia_muni_token.settings');
		$config->set('media_delimiter', $form_state->getValue('media_delimiter'));
		$config->set('media_fields', $form_state->getValue('media_fields'));
		$config->set('reference_delimiter', $form_state->getValue('reference_delimiter'));
		$config->set('reference_fields', $form_state->getValue('reference_fields'));
		$config->set('double_fields', $form_state->getValue('double_fields'));
		$config->set('triple_fields', $form_state->getValue('triple_fields'));
		$config->save();

		return parent::submitForm($form, $form_state);
	}

	/**
	 * {@inheritdoc}
	 */
	protected function getEditableConfigNames()
	{
		return [
			'digitalia_muni_token.settings',
		];
	}
}
