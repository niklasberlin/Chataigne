/*
  ==============================================================================

	ModuleRouterValue.cpp
	Created: 30 Apr 2017 2:59:04pm
	Author:  Ben

  ==============================================================================
*/

#include "ModuleRouterValue.h"
#include "../Module.h"

ModuleRouterValue::ModuleRouterValue(Controllable * _sourceValue, int _index) :
	BaseItem(_sourceValue->niceName),
	valueIndex(_index),
	sourceValue(_sourceValue),
	outModule(nullptr),
	forceDisabled(false)
{
	jassert(sourceValue != nullptr);

	saveAndLoadName = true;
	showInspectorOnSelect = false;

	if (enabled->boolValue())
	{
		if (sourceValue->type == Controllable::TRIGGER) ((Trigger *)sourceValue.get())->addTriggerListener(this);
		else ((Parameter *)sourceValue.get())->addParameterListener(this);
	} else
	{
		if (sourceValue->type == Controllable::TRIGGER) ((Trigger *)sourceValue.get())->removeTriggerListener(this);
		else ((Parameter *)sourceValue.get())->removeParameterListener(this);
	}

	userCanRemove = false;
}

ModuleRouterValue::~ModuleRouterValue()
{
	if (sourceValue == nullptr || sourceValue.wasObjectDeleted()) return;

	if (sourceValue->type == Controllable::TRIGGER) ((Trigger *)sourceValue.get())->removeTriggerListener(this);
	else ((Parameter *)sourceValue.get())->removeParameterListener(this);
}

void ModuleRouterValue::setSourceAndOutModule(Module * s, Module * m)
{
	if (outModule == m) return;

	var prevData = var();

	if (outModule != nullptr)
	{
		if (routeParams != nullptr)
		{
			removeChildControllableContainer(routeParams);
			prevData = routeParams->getJSONData();
		}
		routeParams = nullptr;
	}

	outModule = m;

	if (outModule != nullptr)
	{
		routeParams = outModule->createRouteParamsForSourceValue(s, sourceValue,valueIndex);
		routeParams->loadJSONData(prevData);
		addChildControllableContainer(routeParams);
	}

	valueListeners.call(&ValueListener::routeParamsChanged, this);
}

var ModuleRouterValue::getJSONData()
{
	var data = BaseItem::getJSONData();
	
	data.getDynamicObject()->setProperty("customName", niceName);
	if (sourceValue != nullptr) data.getDynamicObject()->setProperty("niceName", sourceValue->niceName);

	if (routeParams != nullptr) data.getDynamicObject()->setProperty("routeParams", routeParams->getJSONData());
	return data;
}

void ModuleRouterValue::loadJSONDataInternal(var data)
{
	if(sourceValue != nullptr) setNiceName(data.getProperty("niceName", sourceValue->niceName));
	BaseItem::loadJSONDataInternal(data);
	setNiceName(data.getProperty("customName", niceName));
	if (routeParams != nullptr) routeParams->loadJSONData(data.getProperty("routeParams", var()));
} 

void ModuleRouterValue::onContainerParameterChangedInternal(Parameter * p)
{
	if (sourceValue == nullptr || sourceValue.wasObjectDeleted()) return;

	if (p == enabled)
	{
		if (enabled->boolValue())
		{
			if (sourceValue->type == Controllable::TRIGGER) ((Trigger *)sourceValue.get())->addTriggerListener(this);
			else ((Parameter *)sourceValue.get())->addParameterListener(this);
		} else
		{
			if (sourceValue->type == Controllable::TRIGGER) ((Trigger *)sourceValue.get())->removeTriggerListener(this);
			else ((Parameter *)sourceValue.get())->removeParameterListener(this);
		}
	}
}

void ModuleRouterValue::onExternalParameterChanged(Parameter * p)
{
	if (outModule == nullptr) return;
	if (!enabled->boolValue() || forceDisabled) return;
	if(p == sourceValue) outModule->handleRoutedModuleValue(sourceValue, routeParams);
}

void ModuleRouterValue::onExternalTriggerTriggered(Trigger * t)
{
	if (outModule == nullptr) return;
	if (!enabled->boolValue() || forceDisabled) return;
	if(t == sourceValue) outModule->handleRoutedModuleValue(sourceValue, routeParams);
}