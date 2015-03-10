/**
 * @file Stats.h
 * @author Gereon Kremer <gereon.kremer@cs.rwth-aachen.de>
 */

#pragma once

#include <map>
#include <utility>
#include <vector>

#include "../logging.h"
#include "../BenchmarkStatus.h"
#include "Database.h"

namespace benchmax {

class Results {
private:
	std::map<Tool, std::size_t> mTools;
	std::map<fs::path, std::size_t> mFiles;
	std::map<std::pair<std::size_t,std::size_t>, BenchmarkResults> mResults;
public:
	void addResult(const Tool& tool, const fs::path& file, const BenchmarkResults& results) {
		auto toolIt = mTools.find(tool);
		if (toolIt == mTools.end()) {
			toolIt = mTools.emplace(tool, mTools.size()).first;
		}
		auto fileIt = mFiles.find(file);
		if (fileIt == mFiles.end()) {
			fileIt = mFiles.emplace(file, mFiles.size()).first;
		}
		mResults.emplace(std::make_pair(toolIt->second, fileIt->second), results);
	}
	
	void store(Database& db) {
		std::map<std::size_t, std::size_t> toolIDs;
		std::map<std::size_t, std::size_t> fileIDs;
		
		for (const auto& it: mTools) {
			std::cout << "Creating tool " << it.first << std::endl;
			toolIDs[it.second] = db.getToolID(it.first);
			std::cout << toolIDs << std::endl;
		}
		for (const auto& it: mFiles) {
			std::cout << "Creating File " << it.first << std::endl;
			fileIDs[it.second] = db.getFileID(it.first);
		}
		std::size_t benchmarkID = db.createBenchmark();
		for (const auto& it: mResults) {
			std::size_t tool = toolIDs[it.first.first];
			std::size_t file = fileIDs[it.first.second];
			std::cout << "Creating Benchmark " << benchmarkID << std::endl;
			std::size_t id = db.addBenchmarkResult(benchmarkID, tool, file, it.second.exitCode, it.second.time);
			for (const auto& attr: it.second.additional) {
				db.addBenchmarkAttribute(id, attr.first, attr.second);
			}
		}
	}
	
	~Results() {
		std::cout << "Gathered results for " << mTools << std::endl;
		std::cout << "On files " << mFiles << std::endl;
		std::cout << mResults << std::endl;
	}
};

}