#pragma once
#include "ModelConfiguration.h"
#include "core/Configuration.h"
#include <filesystem>
#include <map>
#include <string>
#include <vector>
namespace ventilation::simulation {
struct SweepGrid {
    std::vector<double> airflows{60,80,100,120};
    std::vector<double> extractions{0.10,0.15,0.21,0.30,0.40};
    std::vector<double> ingresses{50,100,150,200,250};
    std::vector<double> targetRHs{60,65,70};
    std::vector<double> minimumTemperatures{8,10,12};
    std::vector<double> powers{200,250,300,400,500};
    std::vector<double> buildingCapacities{1000000,3000000,10000000};
    std::vector<double> wallTemperatures{10,14,18};
    std::vector<double> wallConductances{15,35,75};
};
struct SimulationConfiguration {
    double basementVolumeM3=26.4;
    double nominalAirflowM3h=100;
    double ventilationAirflowM3h=100;
    double moistureIngressGPerHour=100;
    double initialBasementTemperatureC=14;
    double initialBasementRH=80;
    double initialTankLevelLiters=0;
    double ventilationPowerW=0; // Unknown; excluded from electricity total unless configured.
    double tankEmptyIntervalHours=0; // No unrequested tank maintenance.
    double physicalTimeStepSeconds=10;
    double controllerUpdatePeriodSeconds=60;
    double observationFitIngressGPerHour=212;
    double wetSensitivityIngressGPerHour=200;
    double wetSensitivityTankEmptyIntervalHours=24;
    SweepGrid sweeps;
    ThermalParameters thermal;
    DehumidifierParameters dehumidifier;
    WeatherParameters weather;
    Configuration controller;
    SimulationConfiguration();
    bool isValid() const;
};
using IniValues=std::map<std::string,std::string>;
IniValues readIni(const std::filesystem::path& path);
double parseNumber(const std::string& value);
bool parseBoolean(const std::string& value);
SimulationConfiguration loadSimulationConfiguration(const std::filesystem::path& path);
}
