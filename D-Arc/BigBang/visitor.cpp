#include "BigBang/visitor.hpp"
#include "BigBang/cyberspace.hpp"

#include "decorator/border.hpp"
#include "graphlet/ui/textlet.hpp"

#include "datum/string.hpp"

using namespace WarGrey::SCADA;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;

/*************************************************************************************************/
VisitorSpace::VisitorSpace() : Planet("Visitor Space") {}

VisitorSpace::~VisitorSpace() {}

void VisitorSpace::load(CanvasCreateResourcesReason reason, float width, float height) {
	this->space = this->insert_one(new Planetlet(new CyberSpace(), width * 0.618F, height * 0.618F));
	this->space->enable_events(true, true);
}

void VisitorSpace::reflow(float width, float height) {
	this->move_to(this->space, width * 0.5F, height * 0.5F, GraphletAnchor::CC);
}
