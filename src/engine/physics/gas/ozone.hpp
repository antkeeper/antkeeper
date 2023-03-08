/*
 * Copyright (C) 2023  Christopher J. Howard
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

#ifndef ANTKEEPER_PHYSICS_GAS_OZONE_HPP
#define ANTKEEPER_PHYSICS_GAS_OZONE_HPP

#include <engine/math/interpolation.hpp>

namespace physics {
namespace gas {

/// Ozone-related constants and functions.
namespace ozone {

/**
 * Cross sections of ozone from wavelength 280nm to 780nm, in increments of 1nm, at a temperature of 293K, in m-2/molecule.
 *
 * @see https://www.iup.uni-bremen.de/gruppen/molspec/databases/referencespectra/o3spectra2011/index.html
 */
constexpr double cross_sections_280nm_780nm_293k[781 - 280] =
{
	4.08020162329358e-22,
	3.71300005805609e-22,
	3.33329613050011e-22,
	3.08830771169272e-22,
	2.7952452827933e-22,
	2.54507006594824e-22,
	2.31059906830132e-22,
	2.06098663896454e-22,
	1.77641837487505e-22,
	1.60649162987982e-22,
	1.43573256268989e-22,
	1.28028940869356e-22,
	1.11222715354512e-22,
	1.00812840644175e-22,
	8.62757275283743e-23,
	7.74492225274189e-23,
	6.74998572150431e-23,
	5.92584261096169e-23,
	5.09573191001311e-23,
	4.58263122360992e-23,
	3.90608822487099e-23,
	3.4712884682176e-23,
	3.05004947986011e-23,
	2.61948471583207e-23,
	2.35950204633063e-23,
	2.01696279467763e-23,
	1.77716671613151e-23,
	1.57884103804348e-23,
	1.35332466460157e-23,
	1.23148992566335e-23,
	1.00674476685553e-23,
	9.14966132477946e-24,
	8.0373280016411e-24,
	6.82346538467894e-24,
	6.42747947320284e-24,
	5.12300259416845e-24,
	4.77060350699273e-24,
	4.0011431423959e-24,
	3.72184265884461e-24,
	2.71828160861414e-24,
	3.14980819763314e-24,
	2.01180905161078e-24,
	2.23741500460792e-24,
	2.04586872483057e-24,
	1.14774957440809e-24,
	1.7873766001915e-24,
	1.14326155695545e-24,
	7.61393733215954e-25,
	1.30841854222986e-24,
	6.50013011248201e-25,
	4.47253301018733e-25,
	7.22254319278391e-25,
	4.42923420258652e-25,
	3.22168537281097e-25,
	5.60700006481047e-25,
	2.51991724359347e-25,
	1.64975530971913e-25,
	2.69863618345909e-25,
	2.2962216595934e-25,
	1.2768604186372e-25,
	1.6428080690911e-25,
	1.15252741016694e-25,
	6.88123014503947e-26,
	9.01478526179051e-26,
	1.38671804168295e-25,
	7.28648586727799e-26,
	6.3468766437083e-26,
	4.1705109317344e-26,
	3.22298756116943e-26,
	2.52476541455397e-26,
	2.96595291121762e-26,
	2.10981495904366e-26,
	3.0696184107227e-26,
	2.38627682184272e-26,
	1.35645160962203e-26,
	1.14737436955784e-26,
	1.00293019429341e-26,
	1.42699666872085e-26,
	1.03892014298915e-26,
	7.46029500400895e-27,
	7.86729405487508e-27,
	6.49493671023213e-27,
	3.90586420068501e-27,
	3.3969327080211e-27,
	2.69156849765275e-27,
	4.89998022220222e-27,
	4.18363151355665e-27,
	2.41505691668684e-27,
	1.52926807176976e-27,
	1.74010836686791e-27,
	1.43997701486263e-27,
	1.61611242687813e-27,
	1.09444991940235e-27,
	8.68337506495441e-28,
	1.28660044051837e-27,
	1.07534571825705e-27,
	7.59223090396554e-28,
	6.75850905941831e-28,
	6.05594086115429e-28,
	7.40387066310015e-28,
	6.04999720618854e-28,
	5.17923583652293e-28,
	4.0858846895433e-28,
	6.47448369216067e-28,
	5.29992493931534e-28,
	5.63128808710364e-28,
	4.17695119955099e-28,
	5.15330384762416e-28,
	5.25185850011986e-28,
	6.17618171380047e-28,
	6.37119303086457e-28,
	6.8496876006444e-28,
	5.9229625229341e-28,
	5.99998795876176e-28,
	7.06136396740756e-28,
	9.93040926727483e-28,
	1.03668944015898e-27,
	9.60158757124091e-28,
	9.00247643555494e-28,
	1.01801330597124e-27,
	1.0893103182854e-27,
	1.30096252080506e-27,
	1.42018105948242e-27,
	1.53477820553937e-27,
	1.60452920542724e-27,
	1.45613806382105e-27,
	1.63671034304492e-27,
	1.88680147053984e-27,
	2.26659932677371e-27,
	2.57113808884621e-27,
	2.73584069155576e-27,
	2.84524835939992e-27,
	2.65294565386989e-27,
	2.52007318042733e-27,
	2.56347774832346e-27,
	2.73272493525925e-27,
	3.21972108535573e-27,
	3.64879308272645e-27,
	3.86875166703077e-27,
	3.77657059174972e-27,
	3.67917967079418e-27,
	3.84603321414093e-27,
	4.47110813921024e-27,
	5.19879478264214e-27,
	6.13707395634462e-27,
	6.88229484256763e-27,
	7.11935416561536e-27,
	7.15812015493665e-27,
	6.67142397990209e-27,
	6.35218112458219e-27,
	6.34510826220203e-27,
	6.90321809802859e-27,
	7.82871587803871e-27,
	8.52938477234511e-27,
	8.74335964163491e-27,
	8.6718822639457e-27,
	8.59495406258221e-27,
	8.90719500501358e-27,
	9.90176156668504e-27,
	1.14857395824068e-26,
	1.36017282525383e-26,
	1.53232356175871e-26,
	1.75024000506424e-26,
	1.8179765858316e-26,
	1.80631911188605e-26,
	1.70102948254892e-26,
	1.56536231218255e-26,
	1.51141962474665e-26,
	1.57847025841346e-26,
	1.72161452840856e-26,
	1.90909798194127e-26,
	1.96774621921165e-26,
	1.99812678813396e-26,
	1.96900102296014e-26,
	1.95126617395258e-26,
	2.06408915381352e-26,
	2.28866836725858e-26,
	2.57662977048169e-26,
	2.96637551360212e-26,
	3.34197426701549e-26,
	3.73735792971477e-26,
	4.03453827718193e-26,
	4.1291323324037e-26,
	4.07643587970195e-26,
	3.84067732691303e-26,
	3.6271000065179e-26,
	3.50502931147362e-26,
	3.48851626868318e-26,
	3.73778737646723e-26,
	3.97091132121154e-26,
	4.21773978585713e-26,
	4.21620738550362e-26,
	4.22087900183437e-26,
	4.27973060694892e-26,
	4.36010682588909e-26,
	4.60584575680881e-26,
	4.91428506793045e-26,
	5.4918846417406e-26,
	6.10573296817762e-26,
	6.83025566941932e-26,
	7.51469261186479e-26,
	8.08197962688924e-26,
	8.44082645474868e-26,
	8.4843465766735e-26,
	8.4126775729461e-26,
	8.14411830190209e-26,
	7.83636723569755e-26,
	7.60974711104334e-26,
	7.57877917471603e-26,
	7.77887132347866e-26,
	8.07522339055262e-26,
	8.32310316258138e-26,
	8.59015818152486e-26,
	8.67834106505007e-26,
	8.72244226406716e-26,
	8.84734167611993e-26,
	9.0711580597939e-26,
	9.51778147590566e-26,
	1.01490385328969e-25,
	1.09448341447976e-25,
	1.18257044868557e-25,
	1.28105938778111e-25,
	1.37732704252934e-25,
	1.47491161151436e-25,
	1.55090701035304e-25,
	1.60575752538508e-25,
	1.62886093543744e-25,
	1.62802718439262e-25,
	1.60288510229631e-25,
	1.57216917046401e-25,
	1.54475957021763e-25,
	1.534341089264e-25,
	1.5409967492982e-25,
	1.56495784865383e-25,
	1.60012763627009e-25,
	1.63952588489707e-25,
	1.67232066912218e-25,
	1.70167894480834e-25,
	1.7335194060265e-25,
	1.7602731663686e-25,
	1.79953556347172e-25,
	1.84084607422109e-25,
	1.89999243847493e-25,
	1.97490644310087e-25,
	2.05279021301286e-25,
	2.14427839223598e-25,
	2.24098369182092e-25,
	2.34362490982003e-25,
	2.44138366650567e-25,
	2.53826212075759e-25,
	2.62577562731292e-25,
	2.70621837640467e-25,
	2.76622780914432e-25,
	2.80661519633223e-25,
	2.82737499370866e-25,
	2.82968166962191e-25,
	2.82659484597549e-25,
	2.81717325255129e-25,
	2.82197485088881e-25,
	2.84600833360439e-25,
	2.88048754046166e-25,
	2.92686210579676e-25,
	2.98551267943544e-25,
	3.04464034622896e-25,
	3.09724266051229e-25,
	3.14551726028915e-25,
	3.18670379817661e-25,
	3.22330249380314e-25,
	3.25463784914917e-25,
	3.2854797958699e-25,
	3.31405330400124e-25,
	3.34398013095565e-25,
	3.38005272562664e-25,
	3.41218557614901e-25,
	3.45555599852459e-25,
	3.5043277615423e-25,
	3.55984911371566e-25,
	3.6264721972979e-25,
	3.70647322984569e-25,
	3.79188179306458e-25,
	3.88981479760571e-25,
	3.98973596432023e-25,
	4.08527421216539e-25,
	4.17464990288797e-25,
	4.25462181228461e-25,
	4.32317712812093e-25,
	4.38401384845366e-25,
	4.44216978945654e-25,
	4.49508440820886e-25,
	4.5516564927479e-25,
	4.60931329475278e-25,
	4.66173637960526e-25,
	4.70665064920011e-25,
	4.74051362440113e-25,
	4.75812011867871e-25,
	4.7570031038151e-25,
	4.73747927545327e-25,
	4.71027119364443e-25,
	4.66860282624977e-25,
	4.6288533265784e-25,
	4.57997871538082e-25,
	4.53548773884213e-25,
	4.49607272653461e-25,
	4.4568818824566e-25,
	4.42881930827398e-25,
	4.40157474149992e-25,
	4.38677564524713e-25,
	4.37182142194489e-25,
	4.37104565551326e-25,
	4.37919711899623e-25,
	4.39683352146027e-25,
	4.42484514000691e-25,
	4.47212673326828e-25,
	4.53157968174371e-25,
	4.60815452736499e-25,
	4.69376508835705e-25,
	4.78054316070552e-25,
	4.87030272266768e-25,
	4.95220907227656e-25,
	5.02898332230558e-25,
	5.09027352924287e-25,
	5.13210890748271e-25,
	5.15374454317407e-25,
	5.15075653533686e-25,
	5.13171378996911e-25,
	5.09303097809138e-25,
	5.03697164998998e-25,
	4.97218676726656e-25,
	4.89461758141549e-25,
	4.80937594526597e-25,
	4.72371968798565e-25,
	4.63859449875443e-25,
	4.55132403268817e-25,
	4.46787830232533e-25,
	4.38816245012175e-25,
	4.31406743009557e-25,
	4.24483372714822e-25,
	4.17474755876221e-25,
	4.11305082072427e-25,
	4.05554427636835e-25,
	4.0030863998631e-25,
	3.95393149188544e-25,
	3.90576318741963e-25,
	3.86085282288514e-25,
	3.81425489414328e-25,
	3.76584427585746e-25,
	3.71837025816073e-25,
	3.66911165810168e-25,
	3.61941817240908e-25,
	3.56533744970388e-25,
	3.51159798289664e-25,
	3.46140984744989e-25,
	3.41173597486151e-25,
	3.36073006491665e-25,
	3.30753716054224e-25,
	3.25252799457143e-25,
	3.19530424634748e-25,
	3.13651908668849e-25,
	3.07841534489121e-25,
	3.01880275032991e-25,
	2.95965464815758e-25,
	2.90194743931008e-25,
	2.84696394478385e-25,
	2.79488978476361e-25,
	2.74800171431914e-25,
	2.70053589638645e-25,
	2.65588371839819e-25,
	2.6091795190684e-25,
	2.56708069403319e-25,
	2.52375403058723e-25,
	2.48382556862202e-25,
	2.44458617524673e-25,
	2.40594587498642e-25,
	2.36627105488787e-25,
	2.32528309566254e-25,
	2.28436716651676e-25,
	2.24424313328781e-25,
	2.20515093141858e-25,
	2.16647251017334e-25,
	2.12914125517962e-25,
	2.09073684368918e-25,
	2.05335637747553e-25,
	2.01912040845123e-25,
	1.98301051757051e-25,
	1.94465669006103e-25,
	1.9057000954812e-25,
	1.86388414183128e-25,
	1.82241187234978e-25,
	1.78160123951627e-25,
	1.74179167768809e-25,
	1.70080577609513e-25,
	1.6615348247565e-25,
	1.62305192083274e-25,
	1.58738563014106e-25,
	1.55171430112041e-25,
	1.51949383874537e-25,
	1.48607225456117e-25,
	1.45360366466218e-25,
	1.42252183610792e-25,
	1.39462753606039e-25,
	1.36820899679147e-25,
	1.34377836247288e-25,
	1.3233906891166e-25,
	1.30492894377563e-25,
	1.28681831161393e-25,
	1.2663174075999e-25,
	1.2420229295933e-25,
	1.21412305909061e-25,
	1.18502869999655e-25,
	1.15382448376731e-25,
	1.12540171803974e-25,
	1.09558453899584e-25,
	1.06672010609186e-25,
	1.03896476876362e-25,
	1.01172047316729e-25,
	9.84602582159865e-26,
	9.58059205575365e-26,
	9.33099533469407e-26,
	9.08936155224937e-26,
	8.8491955813636e-26,
	8.64349280941688e-26,
	8.44550335501804e-26,
	8.25653238461191e-26,
	8.08238949115334e-26,
	7.94009406468206e-26,
	7.80797137891831e-26,
	7.67364989571709e-26,
	7.54965718671858e-26,
	7.43779079827743e-26,
	7.31964666857758e-26,
	7.2232944312979e-26,
	7.12771524608971e-26,
	7.0510155861502e-26,
	6.99911412369698e-26,
	6.96383131319201e-26,
	6.92914966812787e-26,
	6.85928437919081e-26,
	6.74428511228458e-26,
	6.59224480420957e-26,
	6.38840433633278e-26,
	6.15855599572905e-26,
	5.95772659493798e-26,
	5.76494205198861e-26,
	5.60397496087846e-26,
	5.46017309852595e-26,
	5.32285644696103e-26,
	5.22365816180628e-26,
	5.07899578121548e-26,
	4.93592723237266e-26,
	4.79667132541204e-26,
	4.68132550170927e-26,
	4.56404612656666e-26,
	4.48276475068268e-26,
	4.40173864033052e-26,
	4.35417448301629e-26,
	4.30681941574933e-26,
	4.28871372503407e-26,
	4.24230311436515e-26,
	4.22301315090103e-26,
	4.19685390394596e-26,
	4.18917236558952e-26,
	4.17687017488352e-26,
	4.22019512128238e-26,
	4.2557462015274e-26,
	4.31172890769302e-26,
	4.36741466155527e-26,
	4.41740590419353e-26,
	4.44874945269751e-26,
	4.42029497925774e-26,
	4.34317624813875e-26,
	4.21853858585038e-26,
	4.01907542789597e-26,
	3.80281792816081e-26,
	3.60902775684479e-26,
	3.41921953398109e-26,
	3.25291539681547e-26,
	3.10743399295997e-26,
	2.99157340432027e-26,
	2.89660087626517e-26,
	2.82185391364051e-26,
	2.76872520775773e-26,
	2.72983807771732e-26,
	2.65963957090044e-26,
	2.62737083974039e-26,
	2.56625477748644e-26,
	2.53980768456023e-26,
	2.49759827680602e-26,
	2.47017705195044e-26,
	2.47697548770545e-26,
	2.46973660414457e-26,
	2.47714280793403e-26,
	2.50190654239581e-26,
	2.56196471101148e-26,
	2.64525680175745e-26,
	2.72275203692747e-26,
	2.826728922873e-26,
	2.94507453632992e-26,
	3.05247618840877e-26,
	3.1005889405393e-26,
	3.1508637563266e-26,
	3.13148927506362e-26
};

/**
 * Returns the cross section of ozone at temperature 293k and a given wavelength in the visible spectrum.
 *
 * @param wavelength Wavelength, in nanometers, on `[280, 780)`.
 * @return Ozone cross section at temperature 293k and the given wavelength, in m-2/molecule.
 */
template <class T>
constexpr T cross_section_293k(T wavelength)
{
	int i = static_cast<int>(wavelength);
	int j = static_cast<int>(wavelength + T(1));
	
	if (i < 280 || j > 780)
		return T(0);
	
	const T x = static_cast<T>(cross_sections_280nm_780nm_293k[i - 280]);
	const T y = static_cast<T>(cross_sections_280nm_780nm_293k[j - 280]);
	
	return math::lerp<T>(x, y, wavelength - static_cast<T>(i));
}

/**
 * Calculates an ozone absorption coefficient (wavelength-dependent).
 *
 * @param cross_section Ozone cross section of a wavelength, in m-2/molecule.
 * @param density Molecular number density of ozone, in mol/m-3.
 *
 * @return Ozone absorption coefficient.
 *
 * @see https://skyrenderer.blogspot.com/2012/10/ozone-absorption.html
 */
template <class T>
T absorption(T cross_section, T density)
{
	return cross_section * density;
}

} // namespace ozone

} // namespace gas
} // namespace physics

#endif // ANTKEEPER_PHYSICS_GAS_OZONE_HPP