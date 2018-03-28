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

    return cardStat {0, 0, 0};
}
