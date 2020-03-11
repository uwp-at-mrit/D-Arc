#include "BigBang/IHOS63TDS.hpp"

#include "decorator/border.hpp"

#include "graphlet/filesystem/s63let.hpp"

#include "graphlet/ui/textlet.hpp"
#include "graphlet/ui/statuslet.hpp"

#include "datum/string.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::DTPM;
using namespace WarGrey::Tamer;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;

static const unsigned char test_groups = 2;

static Platform::String^ test_licences[] = { "2a", "2b", "2c", "2d", "2e", "2f"/*, "2g DS1", "2g DS2"*/ };
static Platform::String^ test_validations[] = { "4a", "4b", "4c", "4d", "4e", "4f" };

/*************************************************************************************************/
IHOS63TDS::IHOS63TDS() : Planet("IHO S63 TDS") {}

IHOS63TDS::~IHOS63TDS() {}

void IHOS63TDS::load(CanvasCreateResourcesReason reason, float width, float height) {
	float test_width = width / float(test_groups);
	size_t tl_count = sizeof(test_licences) / sizeof(Platform::String^);
	size_t tv_count = sizeof(test_validations) / sizeof(Platform::String^);

	for (size_t idx = 0; idx < tl_count; idx++) {
		this->load_enchart(test_licences[idx], "2 ENC Licencing/Test " + test_licences[idx], test_width, height / float(tl_count));
	}

	for (size_t idx = 0; idx < tv_count; idx++) {
		this->load_enchart(test_validations[idx], "4 Authentication/Test " + test_validations[idx], test_width, height / float(tv_count));
	}
}

void IHOS63TDS::reflow(float width, float height) {
	float test_width = width / float(test_groups);
	size_t tl_count = sizeof(test_licences) / sizeof(Platform::String^);
	size_t tv_count = sizeof(test_validations) / sizeof(Platform::String^);

	for (size_t idx = 0; idx < tl_count; idx++) {
		this->move_to(this->encs[test_licences[idx]], test_width * 0.0F, height / float(tl_count) * float(idx), GraphletAnchor::LT);
		this->move_to(this->logs[test_licences[idx]], this->encs[test_licences[idx]], GraphletAnchor::LT, GraphletAnchor::LT);
	}

	for (size_t idx = 0; idx < tv_count; idx++) {
		this->move_to(this->encs[test_validations[idx]], test_width * 1.0F, height / float(tv_count) * float(idx), GraphletAnchor::LT);
		this->move_to(this->logs[test_validations[idx]], this->encs[test_validations[idx]], GraphletAnchor::LT, GraphletAnchor::LT);
	}
}

void IHOS63TDS::load_enchart(Platform::String^ logname, Platform::String^ enchart, float width, float height) {
	S63let* s63 = new S63let(enchart, 0x12345U, width, height);
	unsigned int message_count = 4U;
#ifdef _DEBUG
	Statuslinelet* log_recv = new Statuslinelet(Log::Debug, message_count);
#else
	Statuslinelet* log_recv = new Statuslinelet(Log::Info, message_count);
#endif // _DEBUG

	log_recv->fix_width(width);
	this->logs.insert(std::pair<Platform::String^, IGraphlet*>(logname, log_recv));

	if (logname->Equals("2d")) {
		s63->set_pseudo_date(2007, 12, 15);
	} else if (!logname->Equals("2e")){
		s63->set_pseudo_date(2007, 12, 31);
	}

	s63->use_alternative_logger(logname);
	s63->get_logger()->push_log_receiver(log_recv);	
	
	this->insert(log_recv);
	this->encs.insert(std::pair<Platform::String^, Planetlet*>(logname, this->insert_one(s63)));
}
