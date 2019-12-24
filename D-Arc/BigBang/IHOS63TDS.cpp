#include "BigBang/IHOS63TDS.hpp"

#include "decorator/border.hpp"

#include "graphlet/filesystem/s63let.hpp"

#include "graphlet/ui/textlet.hpp"
#include "graphlet/ui/statuslet.hpp"

#include "datum/string.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::Tamer;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;

static Platform::String^ test_licences[] = { "2a", "2b", "2c", "2d", "2e", "2f", "2g DS1", "2g DS2" };

/*************************************************************************************************/
IHOS63TDS::IHOS63TDS() : Planet("IHO S63 TDS") {}

IHOS63TDS::~IHOS63TDS() {}

void IHOS63TDS::load(CanvasCreateResourcesReason reason, float width, float height) {
	size_t tl_count = sizeof(test_licences) / sizeof(Platform::String^);

	for (size_t idx = 0; idx < tl_count; idx++) {
		this->load_enchart(test_licences[idx], "2 ENC Licencing/Test " + test_licences[idx], width, height / float(tl_count));
	}
}

void IHOS63TDS::reflow(float width, float height) {
	size_t tl_count = sizeof(test_licences) / sizeof(Platform::String^);

	for (size_t idx = 0; idx < tl_count; idx++) {
		this->move_to(this->enc_licencing[idx], 0.0F, height / float(tl_count) * float(idx), GraphletAnchor::LT);
		this->move_to(this->enc_licencing_log[idx], this->enc_licencing[idx], GraphletAnchor::LT, GraphletAnchor::LT);
	}
}

void IHOS63TDS::load_enchart(Platform::String^ logname, Platform::String^ enchart, float width, float height) {
	S63let* s63 = new S63let(enchart, 0x12345U, width, height);
	unsigned int message_count = 3U;
#ifdef _DEBUG
	Statuslinelet* log_recv = new Statuslinelet(Log::Debug, message_count);
#elif
	Statuslinelet* log_recv = new Statuslinelet(Log::Info, message_count);
#endif // _DEBUG

	log_recv->fix_width(width);
	this->enc_licencing_log.push_back(log_recv);

	if (logname->Equals("2d")) {
		s63->set_pseudo_date(2007, 12, 15);
	} else if (!logname->Equals("2e")){
		s63->set_pseudo_date(2007, 12, 31);
	}

	s63->use_alternative_logger(logname);
	s63->get_logger()->push_log_receiver(log_recv);	
	
	this->insert(log_recv);
	this->enc_licencing.push_back(this->insert_one(s63));
}
