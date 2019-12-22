#include "CppUnitTest.h"

#include "colorspace.hpp"

using namespace WarGrey::SCADA;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace Windows::UI;

/*************************************************************************************************/
static const unsigned char hue_idx = 3;
static const unsigned char value_idx = 7;
static const unsigned char lightness_idx = 8;
static const unsigned char intensity_idx = 9;
static const unsigned char hsv_s_idx = 11;
static const unsigned char hsl_s_idx = 12;
static const unsigned char hsi_s_idx = 13;

static double samples[][14]{
	/*01*/{ 1.000, 0.000, 0.000,   0.0,   0.0, 1.000, 1.000, 1.000, 0.500, 0.333, 0.299, 1.000, 1.000, 1.000 },
	/*02*/{ 0.750, 0.750, 0.000,  60.0,  60.0, 0.750, 0.750, 0.750, 0.375, 0.500, 0.664, 1.000, 1.000, 1.000 },
	/*03*/{ 0.000, 0.500, 0.000, 120.0, 120.0, 0.500, 0.500, 0.500, 0.250, 0.167, 0.293, 1.000, 1.000, 1.000 },
	/*04*/{ 0.500, 1.000, 1.000, 180.0, 180.0, 0.500, 0.500, 1.000, 0.750, 0.833, 0.850, 0.500, 1.000, 0.400 },
	/*05*/{ 0.500, 0.500, 1.000, 240.0, 240.0, 0.500, 0.500, 1.000, 0.750, 0.667, 0.557, 0.500, 1.000, 0.250 },
	/*06*/{ 0.750, 0.250, 0.750, 300.0, 300.0, 0.500, 0.500, 0.750, 0.500, 0.583, 0.457, 0.667, 0.500, 0.571 },
	/*07*/{ 0.628, 0.643, 0.142,  61.8,  61.5, 0.501, 0.494, 0.643, 0.393, 0.471, 0.581, 0.779, 0.638, 0.699 },
	/*08*/{ 0.255, 0.104, 0.918, 251.1, 250.0, 0.814, 0.750, 0.918, 0.511, 0.426, 0.242, 0.887, 0.832, 0.756 },
	/*09*/{ 0.116, 0.675, 0.255, 134.9, 133.8, 0.559, 0.504, 0.675, 0.396, 0.349, 0.460, 0.828, 0.707, 0.667 },
	/*10*/{ 0.941, 0.785, 0.053,  49.5,  50.5, 0.888, 0.821, 0.941, 0.497, 0.593, 0.748, 0.944, 0.893, 0.911 },
	/*11*/{ 0.704, 0.187, 0.897, 283.7, 284.8, 0.710, 0.636, 0.897, 0.542, 0.596, 0.423, 0.792, 0.775, 0.686 },
	/*12*/{ 0.931, 0.463, 0.316,  14.3,  13.2, 0.615, 0.556, 0.931, 0.624, 0.570, 0.586, 0.661, 0.817, 0.446 },
	/*13*/{ 0.998, 0.974, 0.532,  56.9,  57.4, 0.466, 0.454, 0.998, 0.765, 0.835, 0.931, 0.467, 0.991, 0.363 },
	/*14*/{ 0.099, 0.795, 0.591, 162.4, 163.4, 0.696, 0.620, 0.795, 0.447, 0.495, 0.564, 0.875, 0.779, 0.800 },
	/*15*/{ 0.211, 0.149, 0.597, 248.3, 247.3, 0.448, 0.420, 0.597, 0.373, 0.319, 0.219, 0.750, 0.601, 0.533 },
	/*16*/{ 0.495, 0.493, 0.721, 240.5, 240.4, 0.228, 0.227, 0.721, 0.607, 0.570, 0.520, 0.316, 0.290, 0.135 }
};

/*************************************************************************************************/
namespace WarGrey::Tamer::Graphics {
	private class ColorSpaceConvertion : public TestClass<ColorSpaceConvertion> {
	public:
		ColorSpaceConvertion() {
			for (int i = 0; i < sizeof(samples) / sizeof(double[14]); i++) {
				this->rgbs[i] = rgba(samples[i][0], samples[i][1], samples[i][2]);
			}
		}

	/*
	public:
		TEST_METHOD(RGB_to_HSV) {
			for (int i = 0; i < sizeof(samples) / sizeof(double[14]); i++) {
				double hue, saturation, value;

				fill_hsv_color(rgbs[i], &hue, &saturation, &value);

				//this->assert(hue,        samples[i][hue_idx],   L"[R] RGB ==> HSV");
				//this->assert(saturation, samples[i][hsv_s_idx], L"[G] RGB ==> HSV");
				//this->assert(value,      samples[i][value_idx], L"[B] RGB ==> HSV");
			}
		}

		TEST_METHOD(RGB_to_HSL) {
			for (int i = 0; i < sizeof(samples) / sizeof(double[14]); i++) {
				double hue, saturation, lightness;
				
				fill_hsl_color(rgbs[i], &hue, &saturation, &lightness);

				//this->assert(hue,        samples[i][hue_idx],       L"[R] RGB ==> HSL");
				//this->assert(saturation, samples[i][hsl_s_idx],     L"[G] RGB ==> HSL");
				//this->assert(lightness,  samples[i][lightness_idx], L"[B] RGB ==> HSL");
			}
		}

		TEST_METHOD(RGB_to_HSI) {
			for (int i = 0; i < sizeof(samples) / sizeof(double[14]); i++) {
				double hue, saturation, intensity;

				fill_hsi_color(rgbs[i], &hue, &saturation, &intensity);

				//this->assert(hue,        samples[i][hue_idx],       L"[R] RGB ==> HSI");
				//this->assert(saturation, samples[i][hsi_s_idx],     L"[G] RGB ==> HSI");
				//this->assert(intensity,  samples[i][intensity_idx], L"[B] RGB ==> HSI");
			}
		}
	*/

	public:
		TEST_METHOD(HSV_to_RGB) {
			for (int i = 0; i < sizeof(samples) / sizeof(double[14]); i++) {
				Color hsv = hsva(samples[i][hue_idx], samples[i][hsv_s_idx], samples[i][value_idx]);
				
				this->assert(rgbs[i].R, hsv.R, L"[R] HSV ==> RGB");
				this->assert(rgbs[i].G, hsv.G, L"[G] HSV ==> RGB");
				this->assert(rgbs[i].B, hsv.B, L"[B] HSV ==> RGB");
			}
		}

		TEST_METHOD(HSL_to_RGB) {
			for (int i = 0; i < sizeof(samples) / sizeof(double[14]); i++) {
				Color hsl = hsla(samples[i][hue_idx], samples[i][hsl_s_idx], samples[i][lightness_idx]);

				this->assert(rgbs[i].R, hsl.R, L"[R] HSL ==> RGB");
				this->assert(rgbs[i].G, hsl.G, L"[G] HSL ==> RGB");
				this->assert(rgbs[i].B, hsl.B, L"[B] HSL ==> RGB");
			}
		}

		TEST_METHOD(HSI_to_RGB) {
			for (int i = 0; i < sizeof(samples) / sizeof(double[14]); i++) {
				Color hsi = hsia(samples[i][hue_idx], samples[i][hsi_s_idx], samples[i][intensity_idx]);

				this->assert(rgbs[i].R, hsi.R, L"[R] HSI ==> RGB");
				this->assert(rgbs[i].G, hsi.G, L"[G] HSI ==> RGB");
				this->assert(rgbs[i].B, hsi.B, L"[B] HSI ==> RGB");
			}
		}

	private:
		void assert(unsigned char expected, unsigned char actual, const wchar_t* message) {
			Assert::AreEqual(double(expected), double(actual), 3.0, message);
		}

		void assert(double expected, double actual, const wchar_t* message) {
			Assert::AreEqual(expected, actual, 0.0, message);
		}

	private:
		Color rgbs[sizeof(samples) / sizeof(double[14])];
	};
}
