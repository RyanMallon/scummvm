/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "mohawk/riven_stack.h"

#include "mohawk/cursors.h"
#include "mohawk/riven.h"
#include "mohawk/riven_card.h"
#include "mohawk/riven_graphics.h"
#include "mohawk/riven_video.h"
#include "mohawk/resource.h"

#include "common/events.h"
#include "common/translation.h"

#include "gui/message.h"

namespace Mohawk {

RivenStack::RivenStack(MohawkEngine_Riven *vm, uint16 id) :
		_vm(vm),
		_id(id),
		_mouseIsDown(false),
		_keyPressed(Common::KEYCODE_INVALID) {
	removeTimer();

	loadResourceNames();
	loadCardIdMap();
	setCurrentStackVariable();

	REGISTER_COMMAND(RivenStack, xflies);
}

RivenStack::~RivenStack() {

}

uint16 RivenStack::getId() const {
	return _id;
}

void RivenStack::loadResourceNames() {
	_varNames = RivenNameList(_vm, kVariableNames);
	_externalCommandNames = RivenNameList(_vm, kExternalCommandNames);
	_stackNames = RivenNameList(_vm, kStackNames);
	_cardNames = RivenNameList(_vm, kCardNames);
	_hotspotNames = RivenNameList(_vm, kHotspotNames);
}

Common::String RivenStack::getName(RivenNameResource nameResource, uint16 nameId) const {
	switch (nameResource) {
		case kVariableNames:
			return _varNames.getName(nameId);
		case kExternalCommandNames:
			return _externalCommandNames.getName(nameId);
		case kStackNames:
			return _stackNames.getName(nameId);
		case kCardNames:
			return _cardNames.getName(nameId);
		case kHotspotNames:
			return _hotspotNames.getName(nameId);
		default:
			error("Unknown name resource %d", nameResource);
	}
}

int16 RivenStack::getIdFromName(RivenNameResource nameResource, const Common::String &name) const {
	switch (nameResource) {
		case kVariableNames:
			return _varNames.getNameId(name);
		case kExternalCommandNames:
			return _externalCommandNames.getNameId(name);
		case kStackNames:
			return _stackNames.getNameId(name);
		case kCardNames:
			return _cardNames.getNameId(name);
		case kHotspotNames:
			return _hotspotNames.getNameId(name);
		default:
			error("Unknown name resource %d", nameResource);
	}
}

void RivenStack::loadCardIdMap() {
	Common::SeekableReadStream *rmapStream = _vm->getResource(ID_RMAP, 1);

	uint count = rmapStream->size() / sizeof(uint32);
	_cardIdMap.resize(count);

	for (uint i = 0; i < count; i++) {
		_cardIdMap[i] = rmapStream->readUint32BE();
	}

	delete rmapStream;
}

uint16 RivenStack::getCardStackId(uint32 globalId) const {
	int16 index = -1;

	for (uint16 i = 0; i < _cardIdMap.size(); i++) {
		if (_cardIdMap[i] == globalId)
			index = i;
	}

	if (index < 0)
		error ("Could not match RMAP code %08x", globalId);

	return index;
}

uint32 RivenStack::getCurrentCardGlobalId() const {
	return getCardGlobalId(_vm->getCard()->getId());
}

void RivenStack::setCurrentStackVariable() {
	_vm->_vars["currentstackid"] = _id;
}

uint32 RivenStack::getCardGlobalId(uint16 cardId) const {
	return _cardIdMap[cardId];
}

void RivenStack::dump() const {
	debug("= Stack =");
	debug("id: %d", _id);
	debug("name: %s", RivenStacks::getName(_id));
	debugN("\n");

	for (uint i = 0; i < _cardIdMap.size(); i++) {
		if (!_vm->hasResource(ID_CARD, i)) continue;

		RivenCard *card = new RivenCard(_vm, i);
		card->dump();
		delete card;
	}
}

void RivenStack::runCommand(uint16 commandNameId, const ArgumentArray &args) {
	Common::String externalCommandName = getName(kExternalCommandNames, commandNameId);

	if (!_commands.contains(externalCommandName)) {
		error("Unknown external command \'%s\'", externalCommandName.c_str());
	}

	(*_commands[externalCommandName])(args);
}

void RivenStack::registerCommand(const Common::String &name, ExternalCommand *command) {
	_commands[name] = Common::SharedPtr<ExternalCommand>(command);
}

void RivenStack::xflies(const ArgumentArray &args) {
	_vm->_gfx->setFliesEffect(args[1], args[0] == 1);
}

uint16 RivenStack::getComboDigit(uint32 correctCombo, uint32 digit) {
	static const uint32 powers[] = { 100000, 10000, 1000, 100, 10, 1 };
	return (correctCombo % powers[digit]) / powers[digit + 1];
}

void RivenStack::runDemoBoundaryDialog() {
	GUI::MessageDialog dialog(_("Exploration beyond this point available only within the full version of\n"
			                          "the game."));
	dialog.runModal();
}

void RivenStack::runEndGame(uint16 videoCode, uint32 delay) {
	_vm->_sound->stopAllSLST();
	RivenVideo *video = _vm->_video->openSlot(videoCode);
	video->enable();
	video->play();
	video->setLooping(false);
	runCredits(videoCode, delay);
}

void RivenStack::runCredits(uint16 video, uint32 delay) {
	// Initialize our credits state
	_vm->_cursor->hideCursor();
	_vm->_gfx->beginCredits();
	uint nextCreditsFrameStart = 0;

	RivenVideo *videoPtr = _vm->_video->getSlot(video);

	while (!_vm->hasGameEnded() && _vm->_gfx->getCurCreditsImage() <= 320) {
		if (videoPtr->getCurFrame() >= (int32)videoPtr->getFrameCount() - 1) {
			if (nextCreditsFrameStart == 0) {
				// Set us up to start after delay ms
				nextCreditsFrameStart = _vm->getTotalPlayTime() + delay;
			} else if (_vm->getTotalPlayTime() >= nextCreditsFrameStart) {
				// the first two frames stay on for 4 seconds
				// the rest of the scroll updates happen at 60Hz
				if (_vm->_gfx->getCurCreditsImage() < 304)
					nextCreditsFrameStart = _vm->getTotalPlayTime() + 4000;
				else
					nextCreditsFrameStart = _vm->getTotalPlayTime() + 1000 / 60;

				_vm->_gfx->updateCredits();
			}
		}

		_vm->doFrame();
	}

	_vm->setGameEnded();
}

void RivenStack::installCardTimer() {

}

void RivenStack::onMouseDown(const Common::Point &mouse) {
	_mouseIsDown = true;
	_mousePosition = mouse;

	if (_vm->getCard() && !_vm->_scriptMan->hasQueuedScripts()) {
		_mouseDragStartPosition = mouse;

		RivenScriptPtr script = _vm->getCard()->onMouseDown(mouse);

		if (!script->empty()) {
			_vm->_scriptMan->runScript(script, true);
		}
	}
}

void RivenStack::onMouseUp(const Common::Point &mouse) {
	_mouseIsDown = false;
	_mousePosition = mouse;

	if (_vm->getCard() && !_vm->_scriptMan->hasQueuedScripts()) {
		RivenScriptPtr script = _vm->getCard()->onMouseUp(mouse);

		if (!script->empty()) {
			_vm->_scriptMan->runScript(script, true);
		}
	}
}

void RivenStack::onMouseMove(const Common::Point &mouse) {
	_mousePosition = mouse;

	if (_vm->getCard() && !_vm->_scriptMan->hasQueuedScripts()) {
		RivenScriptPtr script = _vm->getCard()->onMouseMove(mouse);

		if (!script->empty()) {
			_vm->_scriptMan->runScript(script, true);
		}
	}
}

bool RivenStack::mouseIsDown() const {
	return _mouseIsDown;
}

void RivenStack::mouseForceUp() {
	_mouseIsDown = false;
}

void RivenStack::onFrame() {
	if (!_vm->getCard() || _vm->_scriptMan->hasQueuedScripts()) {
		return;
	}

	checkTimer();

	_vm->_gfx->updateEffects();

	RivenScriptPtr script(new RivenScript());
	if (_mouseIsDown) {
		script += _vm->getCard()->onMouseDragUpdate();
	} else {
		script += _vm->getCard()->onFrame();
		script += _vm->getCard()->onMouseUpdate();
	}

	_vm->_scriptMan->runScript(script, true);
}

Common::KeyCode RivenStack::keyGetPressed() const {
	return _keyPressed;
}

void RivenStack::keyForceUp() {
	_keyPressed = Common::KEYCODE_INVALID;
}

void RivenStack::onKeyPressed(const Common::KeyCode keyCode) {
	_keyPressed = keyCode;
}

Common::Point RivenStack::getMousePosition() const {
	return _mousePosition;
}

Common::Point RivenStack::getMouseDragStartPosition() const {
	return _mouseDragStartPosition;
}

void RivenStack::installTimer(TimerProc *proc, uint32 time) {
	removeTimer();
	_timerProc = Common::SharedPtr<TimerProc>(proc);
	_timerTime = time + _vm->getTotalPlayTime();
}

void RivenStack::checkTimer() {
	if (!_timerProc) {
		return;
	}

	// NOTE: If the specified timer function is called, it is its job to remove the timer!

	// Timers are queued as script commands so that they don't run when the doFrame method
	// is called from an inner game loop.
	if (_vm->getTotalPlayTime() >= _timerTime) {
		RivenScriptPtr script = _vm->_scriptMan->createScriptWithCommand(
				new RivenTimerCommand(_vm, _timerProc));
		_vm->_scriptMan->runScript(script, true);
	}
}

void RivenStack::removeTimer() {
	_timerProc.reset();
	_timerTime = 0;
}

bool RivenStack::pageTurn(RivenTransition transition) {
	// Wait until the previous page turn sound completes
	while (_vm->_sound->isEffectPlaying() && !_vm->hasGameEnded()) {
		if (!mouseIsDown()) {
			return false;
		}

		_vm->doFrame();
	}

	// Play the page turning sound
	const char *soundName = nullptr;
	if (_vm->_rnd->getRandomBit())
		soundName = "aPage1";
	else
		soundName = "aPage2";

	_vm->_sound->playCardSound(soundName, 51, true);

	// Now update the screen :)
	_vm->_gfx->scheduleTransition(transition);

	return true;
}

RivenNameList::RivenNameList() {

}

RivenNameList::RivenNameList(MohawkEngine_Riven *vm, uint16 id) {
	loadResource(vm, id);
}

RivenNameList::~RivenNameList() {

}

void RivenNameList::loadResource(MohawkEngine_Riven *vm, uint16 id) {
	Common::SeekableReadStream *nameStream = vm->getResource(ID_NAME, id);

	uint16 namesCount = nameStream->readUint16BE();

	Common::Array<uint16> stringOffsets;
	stringOffsets.resize(namesCount);
	for (uint16 i = 0; i < namesCount; i++) {
		stringOffsets[i] = nameStream->readUint16BE();
	}

	_index.resize(namesCount);
	for (uint16 i = 0; i < namesCount; i++) {
		_index[i] = nameStream->readUint16BE();
	}

	int32 curNamesPos = nameStream->pos();

	_names.resize(namesCount);
	for (uint32 i = 0; i < namesCount; i++) {
		nameStream->seek(curNamesPos + stringOffsets[i]);

		Common::String name;
		for (char c = nameStream->readByte(); c; c = nameStream->readByte())
			name += c;

		_names[i] = name;
	}

	delete nameStream;
}

Common::String RivenNameList::getName(uint16 nameID) const {
	return _names[nameID];
}

int16 RivenNameList::getNameId(const Common::String &name) const {
	int low = 0;
	int high = _index.size() - 1;
	int midpoint = 0;

	// Binary search using the sorted _index array
	while (low <= high)	{
		midpoint = low + (high - low) / 2;

		const Common::String &midpointName = _names[_index[midpoint]];

		int comparison = name.compareToIgnoreCase(midpointName);
		if (comparison == 0) {
			return _index[midpoint];
		} else if (comparison < 0) {
			high = midpoint - 1;
		} else {
			low = midpoint + 1;
		}
	}

	return -1;
}

namespace RivenStacks {
static const char *names[] = {
		"<unknown>",
		"ospit",
		"pspit",
		"rspit",
		"tspit",
		"bspit",
		"gspit",
		"jspit",
		"aspit"
};

const char *getName(uint16 stackId) {
	// Sanity check.
	assert(stackId < ARRAYSIZE(names));

	return names[stackId];
}

uint16 getId(const char *stackName) {
	for (byte i = 0; i < ARRAYSIZE(names); i++) {
		if (scumm_stricmp(stackName, names[i]) == 0) {
			return i;
		}
	}

	return kStackUnknown;
}
} // End of namespace RivenStacks

} // End of namespace Mohawk
