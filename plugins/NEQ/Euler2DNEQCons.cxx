#include "NEQ.hh"
#include "Euler2DNEQCons.hh"
#include "Common/NotImplementedException.hh"
#include "Environment/ObjectProvider.hh"
#include "Framework/PhysicalChemicalLibrary.hh"

//////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace COOLFluiD::Framework;
using namespace COOLFluiD::Common;
using namespace COOLFluiD::Physics::NavierStokes;

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace Physics {

    namespace NEQ {

//////////////////////////////////////////////////////////////////////////////

Environment::ObjectProvider<Euler2DNEQCons, ConvectiveVarSet, NEQModule, 1>
euler2DNEQConsProvider("Euler2DNEQCons");

//////////////////////////////////////////////////////////////////////////////

Euler2DNEQCons::Euler2DNEQCons(Common::SafePtr<BaseTerm> term) :
  MultiScalarVarSet<Euler2DVarSet>(term),
  _library(CFNULL),
  _Rgas(),
  _dhe(3),
  _ys(),
  _rightEv(),
  _leftEv(),
  _alpha(),
  _RiGas(),
  _mmasses(),
  _fcoeff()
{
  const CFuint nbSpecies = getModel()->getNbScalarVars(0);
  const CFuint nbTv = getModel()->getNbScalarVars(1);

  vector<std::string> names(nbSpecies + 3 + nbTv);
  for (CFuint ie = 0; ie < nbSpecies; ++ie) {
    names[ie] = "rho" + StringOps::to_str(ie);
  }
  names[nbSpecies]     = "rhoU";
  names[nbSpecies + 1] = "rhoV";
  names[nbSpecies + 2] = "rhoE";

  if (nbTv > 0){
	const CFuint startTv = nbSpecies + 3;
	for (CFuint ie = 0; ie < nbTv; ++ie) {
	  names[startTv + ie] = "rhoEv" + StringOps::to_str(ie);
	}
  }

  setVarNames(names);
}

//////////////////////////////////////////////////////////////////////////////

Euler2DNEQCons::~Euler2DNEQCons()
{
}

//////////////////////////////////////////////////////////////////////////////

void Euler2DNEQCons::computeEigenValuesVectors(RealMatrix& rightEv,
                                           RealMatrix& leftEv,
                                           RealVector& eValues,
                                           const RealVector& normal)
{
  const CFuint nbSpecies = getModel()->getNbScalarVars(0);
  cf_assert(getModel()->getNbScalarVars(1) == 1);

  const RealVector& lData = getModel()->getPhysicalData();
  SafePtr<PhysicalChemicalLibrary::ExtraData> eData = _library->getExtraData();

  const CFreal T = lData[EulerTerm::T];

  // this is inconsistent with Prabhu's linearization
  //  const CFreal rho = lData[EulerTerm::RHO];
  //const CFreal p = lData[EulerTerm::P];
  //const CFreal cvTr = eData->dEdT;
  //const CFreal beta = p/(rho*T)/cvTr;

  const CFuint firstSpecies = getModel()->getFirstScalarVar(0);
  
  cf_assert(_ys.size() == nbSpecies);
  cf_assert(_alpha.size() == nbSpecies);
  //  cf_assert(p > 0.0);
  cf_assert(T > 0.0);
  //cf_assert(rho > 0.0);

  CFreal numBeta = 0.;
  CFreal denBeta = 0.;
  const CFuint start = (_library->presenceElectron()) ? 1 : 0;
  for (CFuint i = start; i < nbSpecies; ++i) {
    const CFreal sigmai = lData[firstSpecies + i]/_mmasses[i];
    numBeta += sigmai;
    denBeta += sigmai*_fcoeff[i];
  }

  const CFreal beta = numBeta/denBeta;
  for (CFuint is = 0; is < nbSpecies; ++is) {
    _ys[is] = lData[firstSpecies + is];

    if (_ys[is] > 1.1) {
      cout << "_ys > 1.1 = " << _ys << endl;
      // cf_assert(_ys[is] <= 1.1);
    }
  }
  
  CFreal phi = 0.0; 
  if (_library->presenceElectron()) {
    // assume that electrons have 0 as mixture ID  
    phi = _RiGas[0]*_ys[0]/eData->dEvTv - beta;
  }
  else {
    phi =-beta;
  }
  
  CFreal Tq = 0.0;
  for (CFuint is = 0; is < nbSpecies; ++is) {
    if (!_library->presenceElectron()) {
      Tq = T;
    }
    else {
      if (is == 0) {
	Tq = 0.0; // get vibrational temperature
      }
      else{
	Tq = T;
      }
    }
    
    _alpha[is] = _RiGas[is]*Tq - beta*(eData->energyTr)[is]; // change here to include phi
  }
  
  const CFreal nx = normal[XX];
  const CFreal ny = normal[YY];
  const CFreal u = lData[EulerTerm::VX];
  const CFreal v = lData[EulerTerm::VY];

  const CFreal V2 = lData[EulerTerm::V]*lData[EulerTerm::V];
  const CFreal q = 0.5*V2;
  const CFreal V = v*nx -u*ny;
  const CFreal H = lData[EulerTerm::H];
  const CFuint firstTv = getModel()->getFirstScalarVar(1);
  const CFreal ev = lData[firstTv];
  const CFreal a = lData[EulerTerm::A];
  const CFreal a2 = lData[EulerTerm::A]*lData[EulerTerm::A];
  const CFreal U = u*nx + v*ny;

  const CFuint nbSpPlus1 = nbSpecies+1;
  const CFuint nbSpPlus2 = nbSpecies+2;
  const CFuint nbSpPlus3 = nbSpecies+3;

  const CFuint nbTv = getModel()->getNbScalarVars(1);  

  // rightEv = 0.0;
  // leftEv = 0.0;
  for (CFuint is = 0; is < nbSpecies; ++is) {
    rightEv(is, is) = 1.;
    rightEv(is,nbSpPlus1) = 0.5*_ys[is];
    rightEv(is,nbSpPlus2) = 0.5*_ys[is];

    rightEv(nbSpecies,is) = u;
    rightEv(nbSpPlus1,is) = v;
    rightEv(nbSpPlus2,is) = q - _alpha[is]/beta;
  }

  rightEv(nbSpecies,nbSpecies) = -ny*a2;
  rightEv(nbSpecies,nbSpPlus1) = 0.5*(u+a*nx);
  rightEv(nbSpecies,nbSpPlus2) = 0.5*(u-a*nx);

  rightEv(nbSpPlus1,nbSpecies) = nx*a2;
  rightEv(nbSpPlus1,nbSpPlus1) = 0.5*(v+a*ny); // 0.5(v+a*ny) ???
  rightEv(nbSpPlus1,nbSpPlus2) = 0.5*(v-a*ny); // 0.5(v+a*ny) ???

  rightEv(nbSpPlus2,nbSpecies) = V*a2;
  rightEv(nbSpPlus2,nbSpPlus1) = 0.5*(H + a*U);
  rightEv(nbSpPlus2,nbSpPlus2) = 0.5*(H - a*U);
  rightEv(nbSpPlus2,nbSpPlus3) = -phi/beta;
  
  if (nbTv > 0){
	rightEv(nbSpPlus3,nbSpPlus1) = 0.5*ev;
	rightEv(nbSpPlus3,nbSpPlus2) = 0.5*ev;
	
	rightEv(nbSpPlus3,nbSpPlus3) = 1.0;
  }
  
  rightEv /= a2;

  //  cout << "R = "<< endl;
  //   cout << rightEv << endl <<endl;

  const CFreal bq = beta*q;
  for (CFuint is = 0; is < nbSpecies; ++is) {
    for (CFuint js = 0; js < nbSpecies; ++js) {
      const CFreal a2delta = (js != is) ? 0.0 : a2;
      leftEv(is,js) = a2delta - _ys[is]*(_alpha[js] + bq);
    }

    leftEv(is,nbSpecies) = beta*u*_ys[is];
    leftEv(is,nbSpPlus1) = beta*v*_ys[is];
    leftEv(is,nbSpPlus2) = -beta*_ys[is];
	if (nbTv > 0){
	  leftEv(is,nbSpPlus3) = beta*_ys[is];
	}

    const CFreal alphaPlusBetaq = _alpha[is] + bq;
    leftEv(nbSpecies,is) = -V;
    leftEv(nbSpPlus1,is) = alphaPlusBetaq - U*a;
    leftEv(nbSpPlus2,is) = alphaPlusBetaq + U*a;
	if (nbTv > 0){
	  leftEv(nbSpPlus3,is) = -ev*alphaPlusBetaq;\
	}
  }

  leftEv(nbSpecies,nbSpecies) = -ny;
  leftEv(nbSpecies,nbSpPlus1) = nx;

  leftEv(nbSpPlus1,nbSpecies) = a*nx - beta*u; //a*nx - beta*u;
  leftEv(nbSpPlus1,nbSpPlus1) = a*ny - beta*v; //a*ny - beta*v;
  leftEv(nbSpPlus1,nbSpPlus2) = beta;
  if (nbTv > 0){
	leftEv(nbSpPlus1,nbSpPlus3) = phi;
  }

  leftEv(nbSpPlus2,nbSpecies) = -a*nx - beta*u;
  leftEv(nbSpPlus2,nbSpPlus1) = -a*ny - beta*v;
  leftEv(nbSpPlus2,nbSpPlus2) = beta;
  if (nbTv > 0){
	leftEv(nbSpPlus2,nbSpPlus3) = phi;
  }

  if (nbTv > 0){
	leftEv(nbSpPlus3,nbSpecies) = beta*u*ev;
	leftEv(nbSpPlus3,nbSpPlus1) = beta*v*ev;
	leftEv(nbSpPlus3,nbSpPlus2) = -beta*ev;
	leftEv(nbSpPlus3,nbSpPlus3) = a2 - phi*ev;
  }

  eValues = U;
  eValues[nbSpPlus1] += a;
  eValues[nbSpPlus2] -= a;
}

//////////////////////////////////////////////////////////////////////////////

CFuint Euler2DNEQCons::getBlockSeparator() const
{
  return Framework::PhysicalModelStack::getActive()->getNbEq();
}

//////////////////////////////////////////////////////////////////////////////

void Euler2DNEQCons::splitJacobian(RealMatrix& jacobPlus,
                                RealMatrix& jacobMin,
                                RealVector& eValues,
                                const RealVector& normal)
{
  const CFuint nbSpecies = getModel()->getNbScalarVars(0);
  cf_assert(getModel()->getNbScalarVars(1) <= 1);

  const RealVector& lData = getModel()->getPhysicalData();
  SafePtr<PhysicalChemicalLibrary::ExtraData> eData = _library->getExtraData();

  const CFreal T = lData[EulerTerm::T];
  const CFreal rho = lData[EulerTerm::RHO];
  const CFreal p = lData[EulerTerm::P];
  const CFreal cvTr = eData->dEdT;
  const CFreal beta = p/(rho*T)/cvTr;
  const CFuint firstSpecies = getModel()->getFirstScalarVar(0);

  cf_assert(_ys.size() == nbSpecies);
  cf_assert(_alpha.size() == nbSpecies);

  if (p <= 0) {
    cout << "_ys = " << _ys << endl;
    cout << "rho = " << rho << endl;
    cout << "T   = " << T << endl;
    cout << "p   = " << p << endl;
    cout << "cvTr= " << cvTr << endl << endl;
    cf_assert(p > 0.0);
  }

  cf_assert(T > 0.0);
  cf_assert(rho > 0.0);

  for (CFuint is = 0; is < nbSpecies; ++is) {
    _ys[is] = lData[firstSpecies + is];
  }
  
  CFreal phi = 0.0; 
  if (_library->presenceElectron()) {
    // assume that electrons have 0 as mixture ID  
    phi = _RiGas[0]*_ys[0]/eData->dEvTv - beta;
  }
  else {
    phi = -beta;
  }
  
  CFreal Tq = 0.0;
  for (CFuint is = 0; is < nbSpecies; ++is) {
    if (!_library->presenceElectron()) {
      Tq = T;
    }
    else {
      if (is == 0) {
		Tq = 0.0; // get vibrational temperature
      }
    }
    
    _alpha[is] = _RiGas[is]*Tq - beta*(eData->energyTr)[is]; // change here to include phi
  }
  
  const CFreal nx = normal[XX];
  const CFreal ny = normal[YY];
  const CFreal u = lData[EulerTerm::VX];
  const CFreal v = lData[EulerTerm::VY];
  const CFreal V2 = lData[EulerTerm::V]*lData[EulerTerm::V];
  const CFreal q = 0.5*V2;
  const CFreal V = v*nx -u*ny;
  const CFreal H = lData[EulerTerm::H];
  const CFuint firstTv = getModel()->getFirstScalarVar(1);
  const CFreal ev = lData[firstTv];
  const CFreal a2 = (1. + beta)*p/rho; // check Euler2DNEQRhoivtTv: there 'a' is computed in a different way giving slightly diferent results
  const CFreal a = sqrt(a2);
  const CFreal U = u*nx + v*ny;
  const CFuint nbSpPlus1 = nbSpecies+1;
  const CFuint nbSpPlus2 = nbSpecies+2;
  const CFuint nbSpPlus3 = nbSpecies+3;

  const CFuint nbTv = getModel()->getNbScalarVars(1);

  // _rightEv = 0.0;
  // _leftEv = 0.0;
  for (CFuint is = 0; is < nbSpecies; ++is) {
    _rightEv(is, is) = 1.;
    _rightEv(is,nbSpPlus1) = 0.5*_ys[is];
    _rightEv(is,nbSpPlus2) = 0.5*_ys[is];

    _rightEv(nbSpecies,is) = u;
    _rightEv(nbSpPlus1,is) = v;
    _rightEv(nbSpPlus2,is) = q - _alpha[is]/beta;
  }

  _rightEv(nbSpecies,nbSpecies) = -ny*a2;
  _rightEv(nbSpecies,nbSpPlus1) = 0.5*(u+a*nx);
  _rightEv(nbSpecies,nbSpPlus2) = 0.5*(u-a*nx);

  _rightEv(nbSpPlus1,nbSpecies) = nx*a2;
  _rightEv(nbSpPlus1,nbSpPlus1) = 0.5*(v+a*ny); // 0.5(v+a*ny) ???
  _rightEv(nbSpPlus1,nbSpPlus2) = 0.5*(v-a*ny); // 0.5(v+a*ny) ???

  _rightEv(nbSpPlus2,nbSpecies) = V*a2;
  _rightEv(nbSpPlus2,nbSpPlus1) = 0.5*(H + a*U);
  _rightEv(nbSpPlus2,nbSpPlus2) = 0.5*(H - a*U);
  if (nbTv > 0){
	_rightEv(nbSpPlus2,nbSpPlus3) = -phi/beta;
  }

  if (nbTv > 0){
	_rightEv(nbSpPlus3,nbSpPlus1) = 0.5*ev;
	_rightEv(nbSpPlus3,nbSpPlus2) = 0.5*ev;
	_rightEv(nbSpPlus3,nbSpPlus3) = 1.0;
  }

  _rightEv /= a2;

 //  cout << "R = "<< endl;
//   cout << _rightEv << endl <<endl;

  const CFreal bq = beta*q;
  for (CFuint is = 0; is < nbSpecies; ++is) {
    for (CFuint js = 0; js < nbSpecies; ++js) {
      const CFreal a2delta = (js != is) ? 0.0 : a2;
      _leftEv(is,js) = a2delta - _ys[is]*(_alpha[js] + bq);
    }

    _leftEv(is,nbSpecies) = beta*u*_ys[is];
    _leftEv(is,nbSpPlus1) = beta*v*_ys[is];
    _leftEv(is,nbSpPlus2) = -beta*_ys[is];
	if (nbTv > 0){
	  _leftEv(is,nbSpPlus3) = -phi*_ys[is];  
	}

    const CFreal alphaPlusBetaq = _alpha[is] + bq;
    _leftEv(nbSpecies,is) = -V;
    _leftEv(nbSpPlus1,is) = alphaPlusBetaq - U*a;
    _leftEv(nbSpPlus2,is) = alphaPlusBetaq + U*a;
	if (nbTv > 0){
	  _leftEv(nbSpPlus3,is) = -ev*alphaPlusBetaq;
	}
  }

  _leftEv(nbSpecies,nbSpecies) = -ny;
  _leftEv(nbSpecies,nbSpPlus1) = nx;

  _leftEv(nbSpPlus1,nbSpecies) = a*nx - beta*u; //a*nx - beta*u;
  _leftEv(nbSpPlus1,nbSpPlus1) = a*ny - beta*v; //a*ny - beta*v;
  _leftEv(nbSpPlus1,nbSpPlus2) = beta;
  if (nbTv > 0){
	_leftEv(nbSpPlus1,nbSpPlus3) = phi;
  }

  _leftEv(nbSpPlus2,nbSpecies) = -a*nx - beta*u;
  _leftEv(nbSpPlus2,nbSpPlus1) = -a*ny - beta*v;
  _leftEv(nbSpPlus2,nbSpPlus2) = beta;
  if (nbTv > 0){
	_leftEv(nbSpPlus2,nbSpPlus3) = phi;
  }

  if (nbTv > 0){
	_leftEv(nbSpPlus3,nbSpecies) = beta*u*ev;
	_leftEv(nbSpPlus3,nbSpPlus1) = beta*v*ev;
	_leftEv(nbSpPlus3,nbSpPlus2) = -beta*ev;
	_leftEv(nbSpPlus3,nbSpPlus3) = a2 - phi*ev;
  }

  // cout << "L = "<< endl;
  //   cout << _leftEv << endl <<endl;

  // RealMatrix mat(_rightEv.nbRows(), _leftEv.nbRows());
  //   mat = _rightEv*_leftEv;
  //   cout <<"R*L" << endl;
  //   cout << mat << endl << endl;

  //   mat = _leftEv*_rightEv;
  //   cout <<"L*R" << endl;
  //   cout << mat << endl << endl;


  eValues = U;
  eValues[nbSpPlus1] += a;
  eValues[nbSpPlus2] -= a;

//   // jacobian matrix
//   mat = 0.0;
//   for (CFuint is = 0; is < nbSpecies; ++is) {
//     for (CFuint js = 0; js < nbSpecies; ++js) {
//       const CFreal delta = (js != is) ? 0.0 : 1.0;
//       mat(is,js) = (delta - _ys[is])*U;
//     }
//     mat(is,nbSpecies) = _ys[is]*nx;
//     mat(is,nbSpPlus1) = _ys[is]*ny;
//     mat(is,nbSpPlus2) = 0.0;
//     mat(is,nbSpPlus3) = 0.0;

//     const CFreal alphaPlusBeta = _alpha[is] + bq;
//     mat(nbSpecies,is) = alphaPlusBeta*nx -U*u;
//     mat(nbSpPlus1,is) = alphaPlusBeta*ny -U*v;
//     mat(nbSpPlus2,is) = (alphaPlusBeta - H)*U;
//     mat(nbSpPlus3,is) = -U*ev;
//   }

//   mat(nbSpecies,nbSpecies) = (1.-beta)*u*nx+U;
//   mat(nbSpecies,nbSpPlus1) = (1.-beta)*v*nx-V;
//   mat(nbSpecies,nbSpPlus2) = beta*nx;
//   mat(nbSpecies,nbSpPlus3) = phi*nx;

//   mat(nbSpPlus1,nbSpecies) = (1.-beta)*u*ny+V;
//   mat(nbSpPlus1,nbSpPlus1) = (1.-beta)*v*ny+U;
//   mat(nbSpPlus1,nbSpPlus2) = beta*ny;
//   mat(nbSpPlus1,nbSpPlus3) = phi*ny;

//   mat(nbSpPlus2,nbSpecies) = H*nx - beta*U*u;
//   mat(nbSpPlus2,nbSpPlus1) = H*ny - beta*U*v;
//   mat(nbSpPlus2,nbSpPlus2) = (1.+beta)*U;
//   mat(nbSpPlus2,nbSpPlus3) = phi*U;

//   mat(nbSpPlus3,nbSpecies) = ev*nx;
//   mat(nbSpPlus3,nbSpPlus1) = ev*ny;
//   mat(nbSpPlus3,nbSpPlus2) = 0.0;
//   mat(nbSpPlus3,nbSpPlus3) = U;

//   RealMatrix m1(9,9);
//   RealMatrix m2(9,9);

//   m1 = mat*_rightEv;
//   m2 = _leftEv*m1;

//   cout << "lambda M = " << endl;
//   cout << m2 << endl;
//   cout << "lambda = " << eValues << endl << endl;

  // compute the eigen values + and -
  if (_jacobDissip > 0.0) {
    // modified eigenvalues to cure carbuncle
    const CFreal j2 = _jacobDissip*_jacobDissip;
    for (CFuint iEq = 0; iEq < eValues.size(); ++iEq) {
      _eValuesP[iEq] = max(0.,eValues[iEq]);
      const CFreal evP = _eValuesP[iEq];
      _eValuesP[iEq] = 0.5*(evP + sqrt(evP*evP + j2*a2));

      _eValuesM[iEq] = min(0.,eValues[iEq]);
      const CFreal evM = _eValuesM[iEq];
      _eValuesM[iEq] = 0.5*(evM - sqrt(evM*evM + j2*a2));
    }
  }
  else {
    for (CFuint iEq = 0; iEq < eValues.size(); ++iEq) {
      _eValuesP[iEq] = max(0.,eValues[iEq]);
      _eValuesM[iEq] = min(0.,eValues[iEq]);
    }
  }

  // compute jacobian + and -
  jacobPlus = _rightEv*(_eValuesP*_leftEv);
  jacobMin  = _rightEv*(_eValuesM*_leftEv);

  //degugging infos
  CFLogDebugMax( "RightEigenvectors @Euler2DNEQCons::splitJacobian" << "\n"
		 << _rightEv << "\n");
  CFLogDebugMax( "LeftEigenvectors @Euler2DNEQCons::splitJacobian" << "\n"
		 << _leftEv << "\n");
  CFLogDebugMax( "EigenValues @Euler2DNEQCons::splitJacobian" << "\n"
		 << eValues << "\n" << "\n");
}

//////////////////////////////////////////////////////////////////////////////

void Euler2DNEQCons::computePhysicalData(const State& state,
					 RealVector& data)
{
  const CFuint nbSpecies = getModel()->getNbScalarVars(0);
  CFreal rho = 0.0;
  for (CFuint ie = 0; ie < nbSpecies; ++ie) {
    rho += state[ie];
  }

  data[EulerTerm::RHO] = rho;
  const CFreal ovRho = 1./rho;

  // set the species mass fractions
  const CFuint firstSpecies = getModel()->getFirstScalarVar(0);
  for (CFuint ie = 0; ie < nbSpecies; ++ie) {
    _ys[ie] = state[ie]*ovRho;
    data[firstSpecies + ie] = _ys[ie];
  }

  // U  and V Velocity Average
  data[EulerTerm::VX] = state[nbSpecies]*ovRho;
  data[EulerTerm::VY] = state[nbSpecies+1]*ovRho;

  const CFreal V2 = data[EulerTerm::VX]*data[EulerTerm::VX] +
    data[EulerTerm::VY]*data[EulerTerm::VY];
  data[EulerTerm::V] = sqrt(V2);

  const CFuint startTv = getModel()->getFirstScalarVar(1);
  data[startTv]= state[nbSpecies+3]*ovRho; //ev

  data[EulerTerm::E]  = state[nbSpecies+2]*ovRho;
  const CFreal Rgas = _library->getRgas();
  SafePtr<PhysicalChemicalLibrary::ExtraData> eData = _library->getExtraData();

  CFreal denom = 0.;
  CFreal form  = 0.;
  CFreal riovermi  = 0.;
  for (CFuint i = 0; i < nbSpecies; ++i) {
    riovermi += state[i]/_mmasses[i];
    const CFreal yOvM = _ys[i]/_mmasses[i];
    denom += yOvM*((Rgas*_fcoeff[i]));
    form += _ys[i]*eData->enthalpyForm[i];
  }
  
  const CFuint nbTv = getModel()->getNbScalarVars(1);

  if (nbTv > 0){
	data[EulerTerm::T] = (data[EulerTerm::E] - data[startTv] - form-0.5*V2)/denom;
  }
  else{
	data[EulerTerm::T] = (data[EulerTerm::E] -form -0.5*V2)/denom;	
  }

  const CFreal P = data[EulerTerm::T]*Rgas*riovermi;
  data[EulerTerm::P] = P;
  data[EulerTerm::H] = data[EulerTerm::E] + P*ovRho;

  //Speed of Sound  Average
  CFreal numBeta = 0.;
  CFreal denBeta = 0.;
  for (CFuint i = 0; i < nbSpecies; ++i) {
    const CFreal sigmai = _ys[i]/_mmasses[i];
    numBeta += sigmai;
    denBeta += sigmai*_fcoeff[i];
  }

  const CFreal beta = numBeta/denBeta;
  const CFreal RT = Rgas*data[EulerTerm::T];

  CFreal aiyi = 0.0;
  for (CFuint i = 0; i < nbSpecies; ++i) {
    aiyi += (_ys[i]/_mmasses[i])*(RT - beta*(_fcoeff[i]*RT + _mmasses[i]*eData->enthalpyForm[i]));
  }

  if (nbTv > 0){
	data[EulerTerm::A] = std::sqrt(aiyi + beta*(data[EulerTerm::H] - 0.5*V2 - data[startTv]));
  }
  else{
	data[EulerTerm::A] = std::sqrt(aiyi + beta*(data[EulerTerm::H] - 0.5*V2 ));
  }
  // cout << "data = " << data<< endl;
}

//////////////////////////////////////////////////////////////////////////////

void Euler2DNEQCons::computeStateFromPhysicalData(const RealVector& data,
					 State& state)
{
  throw Common::NotImplementedException (FromHere(),"Euler2DNEQCons::computeStateFromPhysicalData()");

  //  state[0] = data[EulerTerm::RHO];
  //   state[1] = data[EulerTerm::RHO]*data[EulerTerm::VX];
  //   state[2] = data[EulerTerm::RHO]*data[EulerTerm::VY];
  //   state[3] = data[EulerTerm::RHO]*data[EulerTerm::E];

  //   // Set the species
  //   const CFuint firstSpecies = getModel()->getFirstScalarVar(0);
  //   const CFuint nbSpecies = getModel()->getNbScalarVars(0);

  //   for (CFuint ie = 0; ie < nbSpecies; ++ie){
  //     state[4 + ie] = data[EulerTerm::RHO]*data[firstSpecies + ie];
  //   }
}

//////////////////////////////////////////////////////////////////////////////

CFreal Euler2DNEQCons::getSpeed(const State& state) const
{
  const CFreal u = state[1]/state[0];
  const CFreal v = state[2]/state[0];
  return sqrt(u*u + v*v);
}

//////////////////////////////////////////////////////////////////////////////

void Euler2DNEQCons::setDimensionalValues(const State& state,
                                          RealVector& result)
{
  throw Common::NotImplementedException
      (FromHere(), "Euler2DNEQCons::setDimensionalValues()");
}

//////////////////////////////////////////////////////////////////////////////

void Euler2DNEQCons::setAdimensionalValues(const Framework::State& state,
                             RealVector& result)
{
  throw Common::NotImplementedException
      (FromHere(), "Euler2DNEQCons::setAdimensionalValues() not implemented");
}

//////////////////////////////////////////////////////////////////////////////

void Euler2DNEQCons::setDimensionalValuesPlusExtraValues
(const State& state, RealVector& result,
 RealVector& extra)
{
  throw Common::NotImplementedException
    (FromHere(), "Euler2DNEQCons::setDimensionalValuesPlusExtraValues()");
}

//////////////////////////////////////////////////////////////////////////////

vector<std::string> Euler2DNEQCons::getExtraVarNames() const
{
  cf_assert(_library.isNotNull());

  vector<std::string> names(3);
  names[0] = "rho";
  names[1] = "H";
  names[2] = "M";

  return names;
}

//////////////////////////////////////////////////////////////////////////////

void Euler2DNEQCons::setup()
{
  MultiScalarVarSet<Euler2DVarSet>::setup();

  const CFuint nbSpecies = getModel()->getNbScalarVars(0);
  // set the equation set data for each of the equation subsets
  // first equation subset
  MultiScalarVarSet<Euler2DVarSet>::getEqSetData().resize(2);
  MultiScalarVarSet<Euler2DVarSet>::getEqSetData()[0].setup(0,0,nbSpecies);

  // second equation subset
  Euler2DVarSet::getEqSetData().resize(1);
  Euler2DVarSet::getEqSetData()[0].setup(1,nbSpecies,3);

  const CFuint nbTv = getModel()->getNbScalarVars(1);
  // third equation subset
  MultiScalarVarSet<Euler2DVarSet>::getEqSetData()[1].setup(2,nbSpecies + 3,nbTv);

  _library = PhysicalModelStack::getActive()->getImplementor()->
    getPhysicalPropertyLibrary<PhysicalChemicalLibrary>();
  cf_assert(_library.isNotNull());

  _Rgas = _library->getRgas();

  _dhe.resize(3 + nbTv);
  _ys.resize(nbSpecies);
  const CFuint totNbEqs = 3 + nbTv + nbSpecies;

  _rightEv.resize(totNbEqs, totNbEqs);
  _rightEv = 0.0;

  _leftEv.resize(totNbEqs, totNbEqs);
  _leftEv = 0.0;

  _alpha.resize(nbSpecies);
  _RiGas.resize(nbSpecies);
  _library->setRiGas(_RiGas);

  // needed for beta coefficient
  _mmasses.resize(nbSpecies);
  _library->getMolarMasses(_mmasses);

  _fcoeff.resize(nbSpecies);

  vector<CFuint> moleculeIDs;
  _library->setMoleculesIDs(moleculeIDs);
  vector<bool> flag(nbSpecies, false);
  for (CFuint i = 0; i < moleculeIDs.size(); ++i) {
    flag[moleculeIDs[i]] = true;
  }

  for (CFuint i = 0; i < nbSpecies; ++i) {
    _fcoeff[i] = (flag[i]) ? 2.5 : 1.5;
  }
}

//////////////////////////////////////////////////////////////////////////////

void Euler2DNEQCons::computePerturbedPhysicalData(const Framework::State& state,
						  const RealVector& pdataBkp,
						  RealVector& pdata,
						  CFuint iVar)
{
  throw Common::NotImplementedException (FromHere(),"Euler2DNEQCons::computePerturbedPhysicalData()");
}

//////////////////////////////////////////////////////////////////////////////

void Euler2DNEQCons::computeProjectedJacobian(const RealVector& normal,
				 RealMatrix& jacob)
{
  const CFuint nbSpecies = getModel()->getNbScalarVars(0);
  cf_assert(getModel()->getNbScalarVars(1) == 1);

  const RealVector& lData = getModel()->getPhysicalData();
  SafePtr<PhysicalChemicalLibrary::ExtraData> eData = _library->getExtraData();

  const CFreal T = lData[EulerTerm::T];
  const CFreal rho = lData[EulerTerm::RHO];
  const CFreal p = lData[EulerTerm::P];
  const CFreal cvTr = eData->dEdT;
  const CFreal beta = p/(rho*T)/cvTr;
  const CFuint firstSpecies = getModel()->getFirstScalarVar(0);
  
  cf_assert(_ys.size() == nbSpecies);
  cf_assert(_alpha.size() == nbSpecies);

  if (p <= 0) {
    cout << "_ys = " << _ys << endl;
    cout << "rho = " << rho << endl;
    cout << "T   = " << T << endl;
    cout << "p   = " << p << endl;
    cout << "cvTr= " << cvTr << endl << endl;
    cf_assert(p > 0.0);
  }

  cf_assert(T > 0.0);
  cf_assert(rho > 0.0);
  
  for (CFuint is = 0; is < nbSpecies; ++is) {
    _ys[is] = lData[firstSpecies + is];
    cf_assert(_ys[is] < 1.1);
  }
  
  CFreal phi = 0.0; 
  if (_library->presenceElectron()) {
    // assume that electrons have 0 as mixture ID  
    phi = _RiGas[0]*_ys[0]/eData->dEvTv - beta;
  }
  else {
    phi =-beta;
  }
   
  CFreal Tq = 0.0;
  for (CFuint is = 0; is < nbSpecies; ++is) {
    if (!_library->presenceElectron()) {
      Tq = T;
    }
    else {
      if (is == 0) {
	    Tq = 0.0; // get vibrational temperature
      }
    }
    
    _alpha[is] = _RiGas[is]*T - beta*(eData->energyTr)[is]; // ad phi
  }

  const CFreal nx = normal[XX];
  const CFreal ny = normal[YY];
  const CFreal u = lData[EulerTerm::VX];
  const CFreal v = lData[EulerTerm::VY];
  const CFreal V2 = lData[EulerTerm::V]*lData[EulerTerm::V];
  const CFreal q = 0.5*V2;
  const CFreal V = v*nx -u*ny;
  const CFreal H = lData[EulerTerm::H];
  const CFuint firstTv = getModel()->getFirstScalarVar(1);
  const CFreal ev = lData[firstTv];
//   const CFreal a2 = (1. + beta)*p/rho;
  const CFreal U = u*nx + v*ny;
  const CFuint nbSpPlus1 = nbSpecies+1;
  const CFuint nbSpPlus2 = nbSpecies+2;
  const CFuint nbSpPlus3 = nbSpecies+3;
  const CFreal bq = beta*q;

  const CFuint nbTv = getModel()->getNbScalarVars(1);
  
  for (CFuint is = 0; is < nbSpecies; ++is) {
    for (CFuint js = 0; js < nbSpecies; ++js) {
      const CFreal delta = (js != is) ? 0.0 : 1.0;
      jacob(is,js) = (delta - _ys[is])*U;
    }
    jacob(is,nbSpecies) = _ys[is]*nx;
    jacob(is,nbSpPlus1) = _ys[is]*ny;
    jacob(is,nbSpPlus2) = 0.0;
	if (nbTv > 0){
	  jacob(is,nbSpPlus3) = 0.0;
	}

    const CFreal alphaPlusBeta = _alpha[is] + bq;
    jacob(nbSpecies,is) = alphaPlusBeta*nx -U*u;
    jacob(nbSpPlus1,is) = alphaPlusBeta*ny -U*v;
    jacob(nbSpPlus2,is) = (alphaPlusBeta - H)*U;
	if (nbTv > 0){
	  jacob(nbSpPlus3,is) = -U*ev;
	}
  }

  jacob(nbSpecies,nbSpecies) = (1.-beta)*u*nx+U;
  jacob(nbSpecies,nbSpPlus1) = (1.-beta)*v*nx-V;
  jacob(nbSpecies,nbSpPlus2) = beta*nx;
  if (nbTv > 0){
	jacob(nbSpecies,nbSpPlus3) = -beta*nx;
  }

  jacob(nbSpPlus1,nbSpecies) = (1.-beta)*u*ny+V;
  jacob(nbSpPlus1,nbSpPlus1) = (1.-beta)*v*ny+U;
  jacob(nbSpPlus1,nbSpPlus2) = beta*ny;
  if (nbTv > 0){
	jacob(nbSpPlus1,nbSpPlus3) = -beta*ny;
  }

  jacob(nbSpPlus2,nbSpecies) = H*nx - beta*U*u;
  jacob(nbSpPlus2,nbSpPlus1) = H*ny - beta*U*v;
  jacob(nbSpPlus2,nbSpPlus2) = (1.+beta)*U;
  if (nbTv > 0){
	jacob(nbSpPlus2,nbSpPlus3) = -beta*U;
  }

  if (nbTv > 0){
	jacob(nbSpPlus3,nbSpecies) = ev*nx;
	jacob(nbSpPlus3,nbSpPlus1) = ev*ny;
	jacob(nbSpPlus3,nbSpPlus2) = 0.0;
	jacob(nbSpPlus3,nbSpPlus3) = U;
  }
}

//////////////////////////////////////////////////////////////////////////////

bool Euler2DNEQCons::isValid(const RealVector& data)
{
  bool correct = true;

  const CFuint nbSpecies = getModel()->getNbScalarVars(0);
  //These are indices!
  const CFuint RHOU = nbSpecies;
  const CFuint RHOV = nbSpecies+1;
  const CFuint RHOE = nbSpecies+2;
  const CFuint RHOEv = nbSpecies+3;
  //

  // needed for beta coefficient
// // note: maybe using _mmasses and _fcoeff is better than molecularMasses and fCoeff
// // That option is available for this class.
  RealVector molecularMasses(nbSpecies);
  _library->getMolarMasses(molecularMasses);
  cout << endl << molecularMasses << endl;

  vector<CFuint> moleculeIDs;
  _library->setMoleculesIDs(moleculeIDs);
  vector<bool> flag(nbSpecies, false);
  for (CFuint i = 0; i < moleculeIDs.size(); ++i) {
    flag[moleculeIDs[i]] = true;
  }

  RealVector fCoeff(nbSpecies);
  for (CFuint i = 0; i < nbSpecies; ++i) {
    fCoeff[i] = (flag[i]) ? 2.5 : 1.5;
  }

  const CFreal Rgas = _library->getRgas();
  SafePtr<PhysicalChemicalLibrary::ExtraData> eData = _library->getExtraData();
  //

  const CFreal rhoU = data[RHOU];
  const CFreal rhoV = data[RHOV];
  const CFreal rhoE = data[RHOE];
  const CFreal rhoEv = data[RHOEv];

  // Compute rho:
  CFreal rho = 0.0;
  for (CFuint ie = 0; ie < nbSpecies; ++ie) {
    rho += data[ie];
  }

  const CFreal ovRho = 1./rho;
  // Internal energy per unit mass E:
  const CFreal E = ( rhoE-(rhoU*rhoU+rhoV*rhoV)*ovRho )*ovRho;
  // Vib. energy per unit mass Ev:
  const CFreal Ev = rhoEv*ovRho;
  //
  const CFreal V2 = (rhoU*rhoU+rhoV*rhoV)*ovRho*ovRho;

  //

  CFreal denom = 0.;
  CFreal form  = 0.;
  CFreal riovermi  = 0.;
  for (CFuint i = 0; i < nbSpecies; ++i) {
    riovermi += data[i]/molecularMasses[i];
    const CFreal yOvM = data[i]/molecularMasses[i];
    denom += yOvM*((Rgas*fCoeff[i]));
    form += data[i]*eData->enthalpyForm[i];
  }

  const CFreal T = (E - Ev - form-0.5*V2)/denom;
  const CFreal p = T*Rgas*riovermi;

  //Compute sound speed:
  CFreal numBeta = 0.;
  CFreal denBeta = 0.;
  for (CFuint i = 0; i < nbSpecies; ++i) {
    const CFreal sigmai = data[i]/molecularMasses[i];
    numBeta += sigmai;
    denBeta += sigmai*fCoeff[i];
  }

  const CFreal beta = numBeta/denBeta;

  const CFreal a = std::sqrt((1+beta)*p*ovRho);
  
  if( ( p < 0.) || (T < 0.) || (a < 0.) ){
  return correct = false;
  }

  // Check positivity of the species:

  for (CFuint ie = 0; ie < nbSpecies; ++ie){
    if( data[ie] < 0. ){
      return correct = false;
    }
  }

return correct;
}
//////////////////////////////////////////////////////////////////////////////

void Euler2DNEQCons::setStateVelocityIDs (std::vector<CFuint>& velIDs)
{
  const CFuint nbSpecies = _library->getNbSpecies();
  velIDs.resize(2); velIDs[XX] = nbSpecies; velIDs[YY] = nbSpecies + 1;
}
      
//////////////////////////////////////////////////////////////////////////////

    } // namespace NEQ

  } // namespace Physics

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////
