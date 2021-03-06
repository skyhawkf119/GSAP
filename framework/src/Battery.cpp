/**  Battery - Body
*   @file       Battery.cpp
*   @ingroup    GSAP-Support
*
*   @brief      Battery model class for prognostics
*
*   @author     Matthew Daigle
*   @version    0.1.0
*
*   @pre        N/A
*
*      Contact: Matthew Daigle (matthew.j.daigle@nasa.gov)
*      Created: March 5, 2016
*
*   @copyright Copyright (c) 2016 United States Government as represented by
*     the Administrator of the National Aeronautics and Space Administration.
*     All Rights Reserved.
*/

#include "Battery.h"

#include <cmath>
#include <vector>

#include "ConfigMap.h"

using namespace PCOE;

// Configuration Keys
const std::string QMOBILE_KEY = "Battery.qMobile";
const std::string RO_KEY = "Battery.Ro";
const std::string VEOD_KEY = "Battery.VEOD";

Battery::Battery() {
    numStates = 8;
    numInputs = 1;
    numOutputs = 2;
    numInputParameters = 2;
    numPredictedOutputs = 1;
    m_dt = 1;
    // Set some default parameters
    setParameters();
}

// Constructor based on configMap
Battery::Battery(const ConfigMap & configMap) : Battery::Battery() {
    if (configMap.includes(QMOBILE_KEY)) {
        setParameters(std::stod(configMap.at(QMOBILE_KEY)[0]));
    }
    if (configMap.includes(RO_KEY)) {
        parameters.Ro = std::stod(configMap.at(RO_KEY)[0]);
    }
    if (configMap.includes(VEOD_KEY)) {
        parameters.VEOD = std::stod(configMap.at(VEOD_KEY)[0]);
    }
}

// Battery State Equation
void Battery::stateEqn(const double, std::vector<double> & x, 
                       const std::vector<double> & u, const std::vector<double> & n, 
                       const double dt) {

    // Extract states
    double Tb = x[0];
    double Vo = x[1];
    double Vsn = x[2];
    double Vsp = x[3];
    double qnB = x[4];
    double qnS = x[5];
    double qpB = x[6];
    double qpS = x[7];

    // Extract inputs
    double P = u[0];

    // Constraints
    double Tbdot = 0;
    double CpBulk = qpB / parameters.VolB;
    double CpSurface = qpS / parameters.VolS;
    double CnSurface = qnS / parameters.VolS;
    double xSn = qnS / parameters.qSMax;
    double xnS = qnS / parameters.qSMax;
    double Ven11 = parameters.An11*(-22 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 10) + pow(2 * xnS - 1, 12)) / parameters.F;
    double Ven1 = parameters.An1*(-2 * xnS*(-xnS + 1) + pow(2 * xnS - 1, 2)) / parameters.F;
    double CnBulk = qnB / parameters.VolB;
    double Ven6 = parameters.An6*(-12 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 5) + pow(2 * xnS - 1, 7)) / parameters.F;
    double xpS = qpS / parameters.qSMax;
    double xSp = qpS / parameters.qBMax;
    double qdotDiffusionBSp = (CpBulk - CpSurface) / parameters.tDiffusion;
    double Ven8 = parameters.An8*(-16 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 7) + pow(2 * xnS - 1, 9)) / parameters.F;
    double Ven7 = parameters.An7*(-14 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 6) + pow(2 * xnS - 1, 8)) / parameters.F;
    double Ven9 = parameters.An9*(-18 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 8) + pow(2 * xnS - 1, 10)) / parameters.F;
    double Ven4 = parameters.An4*(-8 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 3) + pow(2 * xnS - 1, 5)) / parameters.F;
    double Ven3 = parameters.An3*(-6 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 2) + pow(2 * xnS - 1, 4)) / parameters.F;
    double Ven2 = parameters.An2*(-4 * xnS*(-xnS + 1)*(2 * xnS - 1) + pow(2 * xnS - 1, 3)) / parameters.F;
    double qdotDiffusionBSn = (CnBulk - CnSurface) / parameters.tDiffusion;
    double Ven5 = parameters.An5*(-10 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 4) + pow(2 * xnS - 1, 6)) / parameters.F;
    double Vep4 = parameters.Ap4*(-8 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 3) + pow(2 * xpS - 1, 5)) / parameters.F;
    double Vep6 = parameters.Ap6*(-12 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 5) + pow(2 * xpS - 1, 7)) / parameters.F;
    double Vep0 = parameters.Ap0*(2 * xpS - 1) / parameters.F;
    double Vep3 = parameters.Ap3*(-6 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 2) + pow(2 * xpS - 1, 4)) / parameters.F;
    double Vep10 = parameters.Ap10*(-20 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 9) + pow(2 * xpS - 1, 11)) / parameters.F;
    double Vep12 = parameters.Ap12*(-24 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 11) + pow(2 * xpS - 1, 13)) / parameters.F;
    double Jn0 = parameters.kn*pow(xSn, parameters.alpha)*pow(-xSn + 1, parameters.alpha);
    double Vep7 = parameters.Ap7*(-14 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 6) + pow(2 * xpS - 1, 8)) / parameters.F;
    double Vep2 = parameters.Ap2*(-4 * xpS*(-xpS + 1)*(2 * xpS - 1) + pow(2 * xpS - 1, 3)) / parameters.F;
    double Vep11 = parameters.Ap11*(-22 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 10) + pow(2 * xpS - 1, 12)) / parameters.F;
    double Ven0 = parameters.An0*(2 * xnS - 1) / parameters.F;
    double Ven12 = parameters.An12*(-24 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 11) + pow(2 * xnS - 1, 13)) / parameters.F;
    double Ven10 = parameters.An10*(-20 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 9) + pow(2 * xnS - 1, 11)) / parameters.F;
    double Vep9 = parameters.Ap9*(-18 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 8) + pow(2 * xpS - 1, 10)) / parameters.F;
    double Vep5 = parameters.Ap5*(-10 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 4) + pow(2 * xpS - 1, 6)) / parameters.F;
    double Vep8 = parameters.Ap8*(-16 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 7) + pow(2 * xpS - 1, 9)) / parameters.F;
    double Vep1 = parameters.Ap1*(-2 * xpS*(-xpS + 1) + pow(2 * xpS - 1, 2)) / parameters.F;
    double Jp0 = parameters.kp*pow(xSp, parameters.alpha)*pow(-xSp + 1, parameters.alpha);
    double Ven = parameters.U0n + Ven0 + Ven1 + Ven10 + Ven11 + Ven12 + Ven2 + Ven3 + Ven4 + Ven5 + Ven6 + Ven7 + Ven8 + Ven9 + parameters.R*Tb*log((-xnS + 1) / xnS) / parameters.F;
    double Vep = parameters.U0p + Vep0 + Vep1 + Vep10 + Vep11 + Vep12 + Vep2 + Vep3 + Vep4 + Vep5 + Vep6 + Vep7 + Vep8 + Vep9 + parameters.R*Tb*log((-xpS + 1) / xpS) / parameters.F;
    double V = -Ven + Vep - Vo - Vsn - Vsp;
    double i = P / V;
    double qnSdot = -i + qdotDiffusionBSn;
    double Jn = i / parameters.Sn;
    double VoNominal = parameters.Ro*i;
    double Jp = i / parameters.Sp;
    double qnBdot = -qdotDiffusionBSn;
    double qpBdot = -qdotDiffusionBSp;
    double qpSdot = i + qdotDiffusionBSp;
    double Vodot = (-Vo + VoNominal) / parameters.to;
    double VsnNominal = static_cast<double>(parameters.R*Tb*asinh((1.0L / 2.0L)*Jn / Jn0) / (parameters.F*parameters.alpha));
    double VspNominal = static_cast<double>(parameters.R*Tb*asinh((1.0L / 2.0L)*Jp / Jp0) / (parameters.F*parameters.alpha));
    double Vsndot = (-Vsn + VsnNominal) / parameters.tsn;
    double Vspdot = (-Vsp + VspNominal) / parameters.tsp;

    // Update state
    x[0] = Tb + Tbdot*dt;
    x[1] = Vo + Vodot*dt;
    x[2] = Vsn + Vsndot*dt;
    x[3] = Vsp + Vspdot*dt;
    x[4] = qnB + qnBdot*dt;
    x[5] = qnS + qnSdot*dt;
    x[6] = qpB + qpBdot*dt;
    x[7] = qpS + qpSdot*dt;

    // Add process noise
    x[0] += dt*n[0];
    x[1] += dt*n[1];
    x[2] += dt*n[2];
    x[3] += dt*n[3];
    x[4] += dt*n[4];
    x[5] += dt*n[5];
    x[6] += dt*n[6];
    x[7] += dt*n[7];
}

// Battery Output Equation
void Battery::outputEqn(const double, const std::vector<double> & x,
                        const std::vector<double> &, const std::vector<double> & n,
                        std::vector<double> & z) {

    // Extract states
    double Tb = x[0];
    double Vo = x[1];
    double Vsn = x[2];
    double Vsp = x[3];
    //double qnB = x[4];
    double qnS = x[5];
    //double qpB = x[6];
    double qpS = x[7];

    // Extract inputs
    //double P = u[0];

    // Constraints
    double xnS = qnS / parameters.qSMax;
    double Tbm = Tb - 273.15;
    double Ven11 = parameters.An11*(-22 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 10) + pow(2 * xnS - 1, 12)) / parameters.F;
    double xpS = qpS / parameters.qSMax;
    double Vep6 = parameters.Ap6*(-12 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 5) + pow(2 * xpS - 1, 7)) / parameters.F;
    double Ven5 = parameters.An5*(-10 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 4) + pow(2 * xnS - 1, 6)) / parameters.F;
    double Ven7 = parameters.An7*(-14 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 6) + pow(2 * xnS - 1, 8)) / parameters.F;
    double Vep4 = parameters.Ap4*(-8 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 3) + pow(2 * xpS - 1, 5)) / parameters.F;
    double Ven9 = parameters.An9*(-18 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 8) + pow(2 * xnS - 1, 10)) / parameters.F;
    double Ven3 = parameters.An3*(-6 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 2) + pow(2 * xnS - 1, 4)) / parameters.F;
    double Vep7 = parameters.Ap7*(-14 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 6) + pow(2 * xpS - 1, 8)) / parameters.F;
    double Vep11 = parameters.Ap11*(-22 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 10) + pow(2 * xpS - 1, 12)) / parameters.F;
    double Ven12 = parameters.An12*(-24 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 11) + pow(2 * xnS - 1, 13)) / parameters.F;
    double Vep9 = parameters.Ap9*(-18 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 8) + pow(2 * xpS - 1, 10)) / parameters.F;
    double Vep5 = parameters.Ap5*(-10 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 4) + pow(2 * xpS - 1, 6)) / parameters.F;
    double Ven1 = parameters.An1*(-2 * xnS*(-xnS + 1) + pow(2 * xnS - 1, 2)) / parameters.F;
    double Vep8 = parameters.Ap8*(-16 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 7) + pow(2 * xpS - 1, 9)) / parameters.F;
    double Ven6 = parameters.An6*(-12 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 5) + pow(2 * xnS - 1, 7)) / parameters.F;
    double Ven8 = parameters.An8*(-16 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 7) + pow(2 * xnS - 1, 9)) / parameters.F;
    double Vep3 = parameters.Ap3*(-6 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 2) + pow(2 * xpS - 1, 4)) / parameters.F;
    double Vep10 = parameters.Ap10*(-20 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 9) + pow(2 * xpS - 1, 11)) / parameters.F;
    double Vep12 = parameters.Ap12*(-24 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 11) + pow(2 * xpS - 1, 13)) / parameters.F;
    double Ven4 = parameters.An4*(-8 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 3) + pow(2 * xnS - 1, 5)) / parameters.F;
    double Ven2 = parameters.An2*(-4 * xnS*(-xnS + 1)*(2 * xnS - 1) + pow(2 * xnS - 1, 3)) / parameters.F;
    double Vep2 = parameters.Ap2*(-4 * xpS*(-xpS + 1)*(2 * xpS - 1) + pow(2 * xpS - 1, 3)) / parameters.F;
    double Ven0 = parameters.An0*(2 * xnS - 1) / parameters.F;
    double Ven10 = parameters.An10*(-20 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 9) + pow(2 * xnS - 1, 11)) / parameters.F;
    double Vep1 = parameters.Ap1*(-2 * xpS*(-xpS + 1) + pow(2 * xpS - 1, 2)) / parameters.F;
    double Vep0 = parameters.Ap0*(2 * xpS - 1) / parameters.F;
    double Ven = parameters.U0n + Ven0 + Ven1 + Ven10 + Ven11 + Ven12 + Ven2 + Ven3 + Ven4 + Ven5 + Ven6 + Ven7 + Ven8 + Ven9 + parameters.R*Tb*log((-xnS + 1) / xnS) / parameters.F;
    double Vep = parameters.U0p + Vep0 + Vep1 + Vep10 + Vep11 + Vep12 + Vep2 + Vep3 + Vep4 + Vep5 + Vep6 + Vep7 + Vep8 + Vep9 + parameters.R*Tb*log((-xpS + 1) / xpS) / parameters.F;
    double V = -Ven + Vep - Vo - Vsn - Vsp;
    double Vm = V;

    // Set outputs
    z[0] = Tbm;
    z[1] = Vm;

    // Add noise
    z[0] += n[0];
    z[1] += n[1];
}

// Battery Threshold Equation
bool Battery::thresholdEqn(const double t, const std::vector<double> & x, const std::vector<double> & u) {
    // Compute based on voltage, so use output equation to get voltage
    std::vector<double> z(2);
    std::vector<double> zeroNoise(8);
    outputEqn(t, x, u, zeroNoise, z);

    // Determine if voltage (second element in z) is below VEOD threshold
    return z[1] <= parameters.VEOD;
}

// Battery Input Equation
void Battery::inputEqn(const double t, const std::vector<double> & inputParameters, std::vector<double> & u) {
    // Implements variable loading, consisting of a sequence of constant loading portions witch specified magnitude and duration
    // inputParameters contains an even number of elements, pairs of (magnitude,duration)
    if (inputParameters.size() < 2 || inputParameters.size() % 2 != 0) {
        throw std::range_error("Battery::inputEqn - Incorrect number of input parameters");
    }

    // Determine where t lies in the given durations, as this specifies what magnitude to use
    // Here, it is assumed that t and the durations are "consistent", ie, if t0 is 1000 then
    // the durations should be specified relative to that. It would be the responsibility of
    // the user of the battery model to take care of this, since the battery model itself,
    // i.e., the C++ object, does not have any state.
    double elapsedTime = 0;
    for (unsigned int i = 0; i < inputParameters.size(); i += 2) {
        // Update time
        elapsedTime += inputParameters[i + 1];
        // If t hasn't reached elapsedTime yet, this is the portion to use
        if (t <= elapsedTime) {
            u[0] = inputParameters[i];
            return;
        }
    }

    // If we get here, we've run out of portions, so just use the last one
    u[0] = inputParameters[inputParameters.size() - 2];
}

// Battery Predicted Outputs Equation
void Battery::predictedOutputEqn(const double, const std::vector<double> & x, const std::vector<double> &, std::vector<double> & z) {

    // SOC is the only predicted output
    // Compute "nominal" SOC
    double qnS = x[indices.states.qnS];
    double qnB = x[indices.states.qnB];
    double SOC = (qnS + qnB) / parameters.qnMax;
    z[0] = SOC;
}

// Set model parameters, given qMobile
void Battery::setParameters(const double qMobile) {
    // Set qMobile
    parameters.qMobile = qMobile;

    // Set min/max mole fraction and charges
    parameters.xnMax = 0.6;            // maximum mole fraction (neg electrode)
    parameters.xnMin = 0;            // minimum mole fraction (neg electrode)
    parameters.xpMax = 1.0;            // maximum mole fraction (pos electrode)
    parameters.xpMin = 0.4;            // minimum mole fraction (pos electrode) -> note xn+xp=1
    parameters.qMax = parameters.qMobile / (parameters.xnMax - parameters.xnMin);    // note qMax = qn+qp
    parameters.Ro = 0.117215;        // for ohmic drop (current collector resistances plus electrolyte resistance plus solid phase resistances at anode and cathode)

    // constants of nature
    parameters.R = 8.3144621;        // universal gas constant, J/K/mol
    parameters.F = 96487;            // Faraday's constant, C/mol

    // Li-ion parameters
    parameters.alpha = 0.5;            // anodic/cathodic electrochemical transfer coefficient
    parameters.Sn = 0.000437545;    // surface area (- electrode)
    parameters.Sp = 0.00030962;        // surface area (+ electrode)
    parameters.kn = 2120.96;        // lumped constant for BV (- electrode)
    parameters.kp = 248898;            // lumped constant for BV (+ electrode)
    parameters.Vol = 2e-5;            // total interior battery volume/2 (for computing concentrations)
    parameters.VolSFraction = 0.1;    // fraction of total volume occupied by surface volume

    // Volumes (total volume is 2*parameters.Vol), assume volume at each electrode is the
    // same and the surface/bulk split is the same for both electrodes
    parameters.VolS = parameters.VolSFraction*parameters.Vol;  // surface volume
    parameters.VolB = parameters.Vol - parameters.VolS;        // bulk volume

    // Set up charges (Li ions)
    parameters.qpMin = parameters.qMax*parameters.xpMin;                    // min charge at pos electrode
    parameters.qpMax = parameters.qMax*parameters.xpMax;                    // max charge at pos electrode
    parameters.qpSMin = parameters.qpMin*parameters.VolS / parameters.Vol;    // min charge at surface, pos electrode
    parameters.qpBMin = parameters.qpMin*parameters.VolB / parameters.Vol;    // min charge at bulk, pos electrode
    parameters.qpSMax = parameters.qpMax*parameters.VolS / parameters.Vol;    // max charge at surface, pos electrode
    parameters.qpBMax = parameters.qpMax*parameters.VolB / parameters.Vol;    // max charge at bulk, pos electrode
    parameters.qnMin = parameters.qMax*parameters.xnMin;                    // max charge at neg electrode
    parameters.qnMax = parameters.qMax*parameters.xnMax;                    // max charge at neg electrode
    parameters.qnSMax = parameters.qnMax*parameters.VolS / parameters.Vol;    // max charge at surface, neg electrode
    parameters.qnBMax = parameters.qnMax*parameters.VolB / parameters.Vol;    // max charge at bulk, neg electrode
    parameters.qnSMin = parameters.qnMin*parameters.VolS / parameters.Vol;    // min charge at surface, neg electrode
    parameters.qnBMin = parameters.qnMin*parameters.VolB / parameters.Vol;    // min charge at bulk, neg electrode
    parameters.qSMax = parameters.qMax*parameters.VolS / parameters.Vol;        // max charge at surface (pos and neg)
    parameters.qBMax = parameters.qMax*parameters.VolB / parameters.Vol;        // max charge at bulk (pos and neg)

    // time constants
    parameters.tDiffusion = 7e6;        // diffusion time constant (increasing this causes decrease in diffusion rate)
    parameters.to = 6.08671;
    parameters.tsn = 1.00138e3;
    parameters.tsp = 46.4311;

    // Redlich-Kister parameters (positive electrode)
    parameters.U0p = 4.03;
    parameters.Ap0 = -31593.7;
    parameters.Ap1 = 0.106747;
    parameters.Ap2 = 24606.4;
    parameters.Ap3 = -78561.9;
    parameters.Ap4 = 13317.9;
    parameters.Ap5 = 307387;
    parameters.Ap6 = 84916.1;
    parameters.Ap7 = -1.07469e+06;
    parameters.Ap8 = 2285.04;
    parameters.Ap9 = 990894;
    parameters.Ap10 = 283920;
    parameters.Ap11 = -161513;
    parameters.Ap12 = -469218;

    // Redlich-Kister parameters (negative electrode)
    parameters.U0n = 0.01;
    parameters.An0 = 86.19;
    parameters.An1 = 0;
    parameters.An2 = 0;
    parameters.An3 = 0;
    parameters.An4 = 0;
    parameters.An5 = 0;
    parameters.An6 = 0;
    parameters.An7 = 0;
    parameters.An8 = 0;
    parameters.An9 = 0;
    parameters.An10 = 0;
    parameters.An11 = 0;
    parameters.An12 = 0;

    // End-of-discharge voltage threshold
    parameters.VEOD = 3.2;
}

// Initialize state, given an initial voltage, current, and temperature
void Battery::initialize(std::vector<double> & x, const std::vector<double> & u, const std::vector<double> & z) {
    // This is solved via a search procedure
    // Start by setting up an xp and xn vectors
    std::vector<double> xp, xn;
    double xi = 0.4;
    while (xi <= 1.0) {
        xp.push_back(xi);
        xn.push_back(1 - xi);
        xi += 0.0001;
    }

    // Initialize mole fractions
    double xpo = 0.4;
    double xno = 0.6;

    // Compute temperature in K (needed for equations below)
    double Tb = z[indices.outputs.Tbm] + 273.15;

    // Account for voltage drop due to input current (assuming no concentration gradient)
    double voltage = z[indices.outputs.Vm];
    double current = u[indices.inputs.P] / voltage;
    double Vo = current*parameters.Ro;

    // Now, construct the equilibrium potential voltage for each value of xp and xn
    for (size_t i = 0; i < xp.size(); i++) {
        // For xp
        double xpS = xp[i];
        double Vep0 = parameters.Ap0*(2 * xpS - 1) / parameters.F;
        double Vep1 = parameters.Ap1*(-2 * xpS*(-xpS + 1) + pow(2 * xpS - 1, 2)) / parameters.F;
        double Vep2 = parameters.Ap2*(-4 * xpS*(-xpS + 1)*(2 * xpS - 1) + pow(2 * xpS - 1, 3)) / parameters.F;
        double Vep3 = parameters.Ap3*(-6 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 2) + pow(2 * xpS - 1, 4)) / parameters.F;
        double Vep4 = parameters.Ap4*(-8 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 3) + pow(2 * xpS - 1, 5)) / parameters.F;
        double Vep5 = parameters.Ap5*(-10 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 4) + pow(2 * xpS - 1, 6)) / parameters.F;
        double Vep6 = parameters.Ap6*(-12 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 5) + pow(2 * xpS - 1, 7)) / parameters.F;
        double Vep7 = parameters.Ap7*(-14 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 6) + pow(2 * xpS - 1, 8)) / parameters.F;
        double Vep8 = parameters.Ap8*(-16 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 7) + pow(2 * xpS - 1, 9)) / parameters.F;
        double Vep9 = parameters.Ap9*(-18 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 8) + pow(2 * xpS - 1, 10)) / parameters.F;
        double Vep10 = parameters.Ap10*(-20 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 9) + pow(2 * xpS - 1, 11)) / parameters.F;
        double Vep11 = parameters.Ap11*(-22 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 10) + pow(2 * xpS - 1, 12)) / parameters.F;
        double Vep12 = parameters.Ap12*(-24 * xpS*(-xpS + 1)*pow(2 * xpS - 1, 11) + pow(2 * xpS - 1, 13)) / parameters.F;
        double Vep = parameters.U0p + Vep0 + Vep1 + Vep10 + Vep11 + Vep12 + Vep2 + Vep3 + Vep4 + Vep5 + Vep6 + Vep7 + Vep8 + Vep9 + parameters.R*Tb*log((-xpS + 1) / xpS) / parameters.F;
        // For xn
        double xnS = xn[i];
        double Ven0 = parameters.An0*(2 * xnS - 1) / parameters.F;
        double Ven1 = parameters.An1*(-2 * xnS*(-xnS + 1) + pow(2 * xnS - 1, 2)) / parameters.F;
        double Ven2 = parameters.An2*(-4 * xnS*(-xnS + 1)*(2 * xnS - 1) + pow(2 * xnS - 1, 3)) / parameters.F;
        double Ven3 = parameters.An3*(-6 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 2) + pow(2 * xnS - 1, 4)) / parameters.F;
        double Ven4 = parameters.An4*(-8 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 3) + pow(2 * xnS - 1, 5)) / parameters.F;
        double Ven5 = parameters.An5*(-10 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 4) + pow(2 * xnS - 1, 6)) / parameters.F;
        double Ven6 = parameters.An6*(-12 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 5) + pow(2 * xnS - 1, 7)) / parameters.F;
        double Ven7 = parameters.An7*(-14 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 6) + pow(2 * xnS - 1, 8)) / parameters.F;
        double Ven8 = parameters.An8*(-16 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 7) + pow(2 * xnS - 1, 9)) / parameters.F;
        double Ven9 = parameters.An9*(-18 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 8) + pow(2 * xnS - 1, 10)) / parameters.F;
        double Ven10 = parameters.An10*(-20 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 9) + pow(2 * xnS - 1, 11)) / parameters.F;
        double Ven11 = parameters.An11*(-22 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 10) + pow(2 * xnS - 1, 12)) / parameters.F;
        double Ven12 = parameters.An12*(-24 * xnS*(-xnS + 1)*pow(2 * xnS - 1, 11) + pow(2 * xnS - 1, 13)) / parameters.F;
        double Ven = parameters.U0n + Ven0 + Ven1 + Ven10 + Ven11 + Ven12 + Ven2 + Ven3 + Ven4 + Ven5 + Ven6 + Ven7 + Ven8 + Ven9 + parameters.R*Tb*log((-xnS + 1) / xnS) / parameters.F;
        // Compute equilibrium voltage
        double Ve = Vep - Ven;
        // Compute what voltage would be for this xp,xn
        double V = Ve - Vo;
        // In the xp vector, it starts at 0.4 which is fully charged.
        // So, the direction we are searching in is from fully charged to fully discharged.
        // We want to find the first predicted voltage that is less than observed voltage, and this is the xp,xn we want.
        if (V <= voltage) {
            // Set mole fractions
            xpo = xp[i];
            xno = xn[i];
            // Stop the loop
            break;
        }
    }

    // Now, we have found the xp,xn corresponding to the voltage
    // Compute corresponding qS values for these mole fractions
    double qpS0 = parameters.qMax*xpo*parameters.VolS / parameters.Vol;
    double qnS0 = parameters.qMax*xno*parameters.VolS / parameters.Vol;
    // Compute qB values assuming that concentrations are equal (no concentration gradient)
    double qpB0 = qpS0*parameters.VolB / parameters.VolS;
    double qnB0 = qnS0*parameters.VolB / parameters.VolS;

    // Set x
    x[indices.states.Tb] = Tb;
    x[indices.states.Vo] = Vo;
    x[indices.states.Vsn] = 0;
    x[indices.states.Vsp] = 0;
    x[indices.states.qnB] = qnB0;
    x[indices.states.qnS] = qnS0;
    x[indices.states.qpB] = qpB0;
    x[indices.states.qpS] = qpS0;
}
