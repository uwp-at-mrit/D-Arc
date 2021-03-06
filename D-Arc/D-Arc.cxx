﻿#include "application.hxx"
#include "configuration.hpp"

#include "universe.hxx"
#include "navigator/thumbnail.hpp"

#include "syslog.hpp"

#include "BigBang/visitor.hpp"
#include "BigBang/IHOS63TDS.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::GYDM;
using namespace WarGrey::Tamer;

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Background;

using namespace Windows::System;
using namespace Windows::Foundation;

using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Controls;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;

using namespace Microsoft::VisualStudio::TestPlatform::TestExecutor::WinRTCore;

/*************************************************************************************************/
private ref class DArcUniverse : public UniverseDisplay {
public:
	virtual ~DArcUniverse() {}

internal:
	DArcUniverse(Platform::String^ name, IUniverseNavigator* navigator, IHeadUpPlanet* heads_up)
		: UniverseDisplay(make_system_logger(default_logging_level, name), name, navigator, heads_up) {
	}

protected:
	void construct(CanvasCreateResourcesReason reason) override {
		//this->push_planet(new IHOS63TDS());
		this->push_planet(new VisitorSpace());
	}
};

/*************************************************************************************************/
private ref class DArcApplication sealed : public SplitView {
public:
	DArcApplication() : SplitView() {
		this->Margin = ThicknessHelper::FromUniformLength(0.0);
		this->PanePlacement = SplitViewPanePlacement::Left;
		this->DisplayMode = SplitViewDisplayMode::Overlay;
		this->IsPaneOpen = false;

		this->PointerMoved += ref new PointerEventHandler(this, &DArcApplication::on_pointer_moved);
		this->PointerReleased += ref new PointerEventHandler(this, &DArcApplication::on_pointer_released);
	}

public:
	void construct(Platform::String^ name, Size region) {
		Platform::String^ localhost = system_ipv4_address();
		IUniverseNavigator* navigator = new ThumbnailNavigator(default_logging_level, name, region.Width / region.Height, 160.0F);

		this->universe = ref new DArcUniverse(name, navigator, nullptr);
		
		// TODO: Why SplitView::Content cannot do it on its own?
		this->universe->register_virtual_keydown_event_handler(this);

		this->Content = this->universe->canvas;
		this->timeline = ref new CompositeTimerListener();
		this->timer = ref new Timer(this->timeline, frame_per_second);
		this->timeline->push_timer_listener(this->universe);

		{ // construct the functional panel
			StackPanel^ panel = ref new StackPanel();

			this->universe->navigator->min_height(region.Height * 0.85F);

			panel->Orientation = ::Orientation::Vertical;
			panel->HorizontalAlignment = ::HorizontalAlignment::Stretch;
			panel->VerticalAlignment = ::VerticalAlignment::Stretch;

			panel->Children->Append(this->universe->navigator->user_interface());

			this->Pane = panel;
			this->OpenPaneLength = this->universe->navigator->min_width();
		}
	}

	void on_foreground_activated(Platform::String^ arguments) {
		UnitTestClient::Run(arguments);
	}
	
	void on_entered_background(EnteredBackgroundEventArgs^ args) {}
	void on_background_activated(IBackgroundTaskInstance^ task) {}
	void on_leaving_background(LeavingBackgroundEventArgs^ args) {}
	void on_suspending(SuspendingEventArgs^ args) {}
	void on_resuming() {}
	
private:
	void on_pointer_moved(Platform::Object^ sender, PointerRoutedEventArgs^ args) {
		auto pt = args->GetCurrentPoint(this);
		float x = pt->Position.X;

		if (!pt->Properties->IsLeftButtonPressed) {
			if (x <= this->Margin.Left) {
				this->IsPaneOpen = true;
				args->Handled = true;
			} 
		}
	}

	void on_pointer_released(Platform::Object^ sender, PointerRoutedEventArgs^ args) {
		auto pt = args->GetCurrentPoint(this);
		float x = pt->Position.X;

		if (pt->Properties->PointerUpdateKind == PointerUpdateKind::LeftButtonReleased) {
			if (x <= normal_font_size) {
				this->IsPaneOpen = true;
				args->Handled = true;
			}
		}
	}

private:
	CompositeTimerListener^ timeline;
	Timer^ timer;

private:
	DArcUniverse^ universe;
};

int main(Platform::Array<Platform::String^>^ args) {
	return launch_universal_windows_application<DArcApplication>(default_logging_level, rsyslog_host, rsyslog_port);
}
