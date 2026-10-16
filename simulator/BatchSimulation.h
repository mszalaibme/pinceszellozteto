#pragma once
#include "SimulationRunner.h"
#include <ostream>
namespace ventilation::simulation {
struct Experiment { std::string group,id; Scenario scenario; SimulationConfiguration configuration; double hours=168; };
std::vector<Experiment> baselineExperiments(const SimulationConfiguration& base,double hours=168);
std::vector<Experiment> sensitivityExperiments(const SimulationConfiguration& base,double hours=168);
void writeSummaryHeader(std::ostream& stream);
void writeSummaryRow(std::ostream& stream,const SimulationResult& result,const std::string& group);
void writeSweepReport(const std::vector<std::pair<std::string,SimulationResult>>& results,const std::filesystem::path& path);
void runBatch(const SimulationConfiguration& base,const std::filesystem::path& output,double step=60,double hours=168,bool baselineOnly=false);
}
