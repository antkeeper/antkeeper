/*
 * Copyright (C) 2020  Christopher J. Howard
 *
 * This file is part of Antkeeper source code.
 *
 * Antkeeper source code is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Antkeeper source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Antkeeper source code.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "celestial-mechanics.hpp"
#include "math/angles.hpp"
#include <cmath>

namespace ast
{

double approx_ecliptic_obliquity(double jd)
{
	return math::radians<double>(23.4393 - 3.563e-7 * (jd - 2451545.0));
}

double3 approx_sun_ecliptic(double jd)
{
	const double t = (jd - 2451545.0) / 36525.0;
	const double m = 6.24 + 628.302 * t;
	
	const double longitude = 4.895048 + 628.331951 * t + (0.033417 - 0.000084 * t) * std::sin(m) + 0.000351 * std::sin(m * 2.0);
	const double latitude = 0.0;
	const double distance = 1.000140 - (0.016708 - 0.000042 * t) * std::cos(m) - 0.000141 * std::cos(m * 2.0);
	
	double3 ecliptic;
	ecliptic.x = distance * std::cos(longitude) * std::cos(latitude);
	ecliptic.y = distance * std::sin(longitude) * std::cos(latitude);
	ecliptic.z = distance * std::sin(latitude);
	
	return ecliptic;
}

double3 approx_moon_ecliptic(double jd)
{
	const double t = (jd - 2451545.0) / 36525.0;
	const double l1 = 3.8104 + 8399.7091 * t;
	const double m1 = 2.3554 + 8328.6911 * t;
	const double m = 6.2300 + 628.3019 * t;
	const double d = 5.1985 + 7771.3772 * t;
	const double d2 = d * 2.0;
	const double f = 1.6280 + 8433.4663 * t;
	
	const double longitude = l1
		+ 0.1098 * std::sin(m1)
		+ 0.0222 * std::sin(d2 - m1)
		+ 0.0115 * std::sin(d2)
		+ 0.0037 * std::sin(m1 * 2.0)
		- 0.0032 * std::sin(m)
		- 0.0020 * std::sin(d2)
		+ 0.0010 * std::sin(d2 - m1 * 2.0)
		+ 0.0010 * std::sin(d2 - m - m1)
		+ 0.0009 * std::sin(d2 + m1)
		+ 0.0008 * std::sin(d2 - m)
		+ 0.0007 * std::sin(m1 - m)
		- 0.0006 * std::sin(d)
		- 0.0005 * std::sin(m + m1);
	
	const double latitude = 0.0895 * sin(f)
		+ 0.0049 * std::sin(m1 + f)
		+ 0.0048 * std::sin(m1 - f)
		+ 0.0030 * std::sin(d2 - f)
		+ 0.0010 * std::sin(d2 + f - m1)
		+ 0.0008 * std::sin(d2 - f - m1)
		+ 0.0006 * std::sin(d2 + f);
	
	const double r = 1.0 / (0.016593
		+ 0.000904 * std::cos(m1)
		+ 0.000166 * std::cos(d2 - m1)
		+ 0.000137 * std::cos(d2)
		+ 0.000049 * std::cos(m1 * 2.0)
		+ 0.000015 * std::cos(d2 + m1)
		+ 0.000009 * std::cos(d2 - m));
	
	double3 ecliptic;
	ecliptic.x = r * std::cos(longitude) * std::cos(latitude);
	ecliptic.y = r * std::sin(longitude) * std::cos(latitude);
	ecliptic.z = r * std::sin(latitude);
	
	return ecliptic;
}

double3x3 approx_moon_ecliptic_rotation(double jd)
{
	const double t = (jd - 2451545.0) / 36525.0;
	const double l1 = 3.8104 + 8399.7091 * t;
	const double f = 1.6280 + 8433.4663 * t;
	
	const double az0 = f + math::pi<double>;
	const double ax  = 0.026920;
	const double az1 = l1 - f;
	
	double3x3 rz0 =
	{
		cos(az0), -sin(az0), 0,
		sin(az0), cos(az0), 0,
		0, 0, 1
	};
	
	double3x3 rx =
	{
		1, 0, 0,
		0, cos(ax), -sin(ax),
		0, sin(ax), cos(ax)
	};
	
	double3x3 rz1 =
	{
		cos(az1), -sin(az1), 0,
		sin(az1), cos(az1), 0,
		0, 0, 1
	};
	
	return rz0 * rx * rz1;
}

double3 solve_kepler(const kepler_orbit& orbit, double t)
{
	// Orbital period (Kepler's third law)
	double pr = std::sqrt(orbit.a * orbit.a * orbit.a);
	
	// Mean anomaly
	double epoch = 0.0;
	double ma = (math::two_pi<double> * (t - epoch)) / pr;
	
	// Semi-minor axis
	const double b = std::sqrt(1.0 - orbit.ec * orbit.ec);
	
	// Trig of orbital elements (can be precalculated?)
	const double cos_ma = std::cos(ma);
	const double sin_ma = std::sin(ma);
	const double cos_om = std::cos(orbit.om);
	const double sin_om = std::sin(orbit.om);
	const double cos_i = std::cos(orbit.i);
	const double sin_i = std::sin(orbit.i);
	
	// Eccentric anomaly
	double ea = ma + orbit.ec * sin_ma * (1.0 + orbit.ec * cos_ma);
	
	// Find radial distance (r) and true anomaly (v)
	double x = orbit.a * (std::cos(ea) - orbit.ec);
	double y = b * std::sin(ea);
	double r = std::sqrt(x * x + y * y);
	double v = std::atan2(y, x);
	
	// Convert (r, v) to ecliptic rectangular coordinates
	double cos_wv = std::cos(orbit.w + v);
	double sin_wv = std::sin(orbit.w + v);
	return double3
	{
		r * (cos_om * cos_wv - sin_om * sin_wv * cos_i),
		r * (sin_om * cos_wv + cos_om * sin_wv * cos_i),
		r * sin_wv * sin_i
	};
}

double3 solve_kepler(double a, double ec, double w, double ma, double i, double om)
{
	// Semi-minor axis
	double b = std::sqrt(1.0 - ec * ec);
	
	// Eccentric anomaly
	double ea = ma + ec * std::sin(ma) * (1.0 + ec * std::cos(ma));
	
	// Find radial distance (r) and true anomaly (v)
	double x = a * (std::cos(ea) - ec);
	double y = b * std::sin(ea);
	double r = std::sqrt(x * x + y * y);
	double v = std::atan2(y, x);
	
	// Convert (r, v) to ecliptic rectangular coordinates
	double cos_om = std::cos(om);
	double sin_om = std::sin(om);
	double cos_i = std::cos(i);
	double cos_wv = std::cos(w + v);
	double sin_wv = std::sin(w + v);
	return double3
	{
		r * (cos_om * cos_wv - sin_om * sin_wv * cos_i),
		r * (sin_om * cos_wv + cos_om * sin_wv * cos_i),
		r * sin_wv * std::sin(i)
	};
}

} // namespace ast
