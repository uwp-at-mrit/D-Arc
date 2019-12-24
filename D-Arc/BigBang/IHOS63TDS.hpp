#pragma once

#include "planet.hpp"
#include "graphlet/planetlet.hpp"

namespace WarGrey::Tamer {
	private class IHOS63TDS : public WarGrey::SCADA::Planet {
	public:
		virtual ~IHOS63TDS() noexcept;
		IHOS63TDS();

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;

	private:
		void load_enchart(Platform::String^ logname, Platform::String^ enchart, float width, float height);

	private: // never deletes these graphlets manually
		std::map<Platform::String^, WarGrey::SCADA::Planetlet*> encs;
		std::map<Platform::String^, WarGrey::SCADA::IGraphlet*> logs;
	};
}
