//
// Created by Nikolay Yakovets on 2018-02-01.
//

#include "SimpleGraph.h"
#include "SimpleEstimator.h"

SimpleEstimator::SimpleEstimator(std::shared_ptr<SimpleGraph> &g){

    // works only with SimpleGraph
    graph = g;
}

void SimpleEstimator::prepare() {

    // do your prep here
    this->labelOccurences.resize(graph->getNoLabels());
    for (auto start_node : graph->adj) {
      for (auto edge : start_node) {
        auto label = edge.first;
        labelOccurences[label]++;
      }
    }
}

cardStat SimpleEstimator::estimate(RPQTree *q) {

    // perform your estimation here

    // Is intended to be used with *one* label only. Not sure
    // how it works for multiple labels in a query.
    std::regex labelPat (R"((\d+))");

    double estimation = 1;
    auto nodes = SimpleGraph::inOrderNodesClean(q);

    std::smatch matches;
    for (auto n : nodes) {
      if (std::regex_search(n, matches, labelPat)) {
        auto label = std::stoul(matches[1]);
        estimation *= labelOccurences[label];
      }
    }
    return cardStat {0, (uint32_t) log(estimation), 0};
}
