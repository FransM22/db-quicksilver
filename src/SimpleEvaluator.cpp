//
// Created by Nikolay Yakovets on 2018-02-02.
//

#include "SimpleEstimator.h"
#include "SimpleEvaluator.h"

SimpleEvaluator::SimpleEvaluator(std::shared_ptr<SimpleGraph> &g) {

    // works only with SimpleGraph
    graph = g;
    est = nullptr; // estimator not attached by default
}

void SimpleEvaluator::attachEstimator(std::shared_ptr<SimpleEstimator> &e) {
    est = e;
}

void SimpleEvaluator::prepare() {

    // if attached, prepare the estimator
    if(est != nullptr) est->prepare();

    // prepare other things here.., if necessary

}

cardStat SimpleEvaluator::computeStats(std::shared_ptr<SimpleGraph> &g) {

    cardStat stats {};

    for(int source = 0; source < g->getNoVertices(); source++) {
        if(!g->adj[source].empty()) stats.noOut++;
    }

    stats.noPaths = g->getNoDistinctEdges();

    for(int target = 0; target < g->getNoVertices(); target++) {
        if(!g->reverse_adj[target].empty()) stats.noIn++;
    }

    return stats;
}

std::shared_ptr<SimpleGraph> SimpleEvaluator::project(uint32_t projectLabel, bool inverse, std::shared_ptr<SimpleGraph> &in) {

    auto out = std::make_shared<SimpleGraph>(in->getNoVertices());
    out->setNoLabels(in->getNoLabels());

    if(!inverse) {
        // going forward
        for(uint32_t source = 0; source < in->getNoVertices(); source++) {
            for (auto labelTarget : in->adj[source]) {

                auto label = labelTarget.first;
                auto target = labelTarget.second;

                if (label == projectLabel)
                    out->addEdge(source, target, label);
            }
        }
    } else {
        // going backward
        for(uint32_t source = 0; source < in->getNoVertices(); source++) {
            for (auto labelTarget : in->reverse_adj[source]) {

                auto label = labelTarget.first;
                auto target = labelTarget.second;

                if (label == projectLabel)
                    out->addEdge(source, target, label);
            }
        }
    }

    return out;
}

std::shared_ptr<SimpleGraph> SimpleEvaluator::join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right) {

    auto out = std::make_shared<SimpleGraph>(left->getNoVertices());
    out->setNoLabels(1);

    for(uint32_t leftSource = 0; leftSource < left->getNoVertices(); leftSource++) {
        for (auto labelTarget : left->adj[leftSource]) {

            int leftTarget = labelTarget.second;
            // try to join the left target with right source
            for (auto rightLabelTarget : right->adj[leftTarget]) {

                auto rightTarget = rightLabelTarget.second;
                out->addEdge(leftSource, rightTarget, 0);

            }
        }
    }

    return out;
}

std::shared_ptr<SimpleGraph> SimpleEvaluator::evaluate_aux(RPQTree *q) {

    // evaluate according to the AST bottom-up

    if(q->isLeaf()) {
        // project out the label in the AST
        std::regex directLabel (R"((\d+)\+)");
        std::regex inverseLabel (R"((\d+)\-)");

        std::smatch matches;

        uint32_t label;
        bool inverse;

        if(std::regex_search(q->data, matches, directLabel)) {
            label = (uint32_t) std::stoul(matches[1]);
            inverse = false;
        } else if(std::regex_search(q->data, matches, inverseLabel)) {
            label = (uint32_t) std::stoul(matches[1]);
            inverse = true;
        } else {
            std::cerr << "Label parsing failed!" << std::endl;
            return nullptr;
        }

        return SimpleEvaluator::project(label, inverse, graph);
    }

    if(q->isConcat()) {

        // evaluate the children
        auto leftGraph = SimpleEvaluator::evaluate_aux(q->left);
        auto rightGraph = SimpleEvaluator::evaluate_aux(q->right);

        // join left with right
        return SimpleEvaluator::join(leftGraph, rightGraph);

    }

    return nullptr;
}

cardStat SimpleEvaluator::evaluate(RPQTree *query) {
    auto a = getPrioritizedAST(query);

    auto res = evaluate_aux(query);
    return SimpleEvaluator::computeStats(res);
}

RPQTree* SimpleEvaluator::getPrioritizedAST(RPQTree *query) {
  auto queryParts = SimpleGraph::inOrderNodesClean(query);
  auto queryString = getPrioritizedAST_aux(queryParts)[0];
  return RPQTree::strToTree(queryString);
}

std::vector<std::string> SimpleEvaluator::getPrioritizedAST_aux(std::vector<std::string> queryParts) {
  if (queryParts.size() == 1) {
    return queryParts;
  }

  std::vector<int> cardinalities;
  cardinalities.resize(queryParts.size()-1);

  for (auto i = 1; i < queryParts.size(); i++) {
    std::string query = queryParts[i-1] + "/" + queryParts[i];
    auto queryTree = RPQTree::strToTree(query);
    cardinalities[i-1] = est->estimate(queryTree).noPaths;
  }

  int min_cardinality = 9999999;
  int min_cardinality_i;

  for (auto i = 1; i < queryParts.size(); i++) {
    auto cardinality = cardinalities[i-1];
    if (cardinality < min_cardinality) {
      min_cardinality_i = i;
      min_cardinality = cardinality;
    }
  }

  std::vector<std::string> newQueryParts;
  newQueryParts.resize(queryParts.size() - 1);

  for (auto i = 0; i < newQueryParts.size(); i++) {
    if (i < min_cardinality_i) {
      newQueryParts[i] = queryParts[i];
    } else if (i == min_cardinality_i) {
      newQueryParts[i] = "(" + queryParts[i] + "/" + queryParts[i+1] + ")";
    } else if (i >= min_cardinality_i + 1) {
      newQueryParts[i] = queryParts[i+1];
    }
  }
  return getPrioritizedAST_aux(newQueryParts);
}
