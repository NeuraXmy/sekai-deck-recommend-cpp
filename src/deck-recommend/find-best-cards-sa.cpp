#include "deck-recommend/base-deck-recommend.h"


void BaseDeckRecommend::findBestCardsSA(
    int liveType,
    const DeckRecommendConfig& cfg,
    Rng& rng,
    const std::vector<CardDetail> &cardDetails,     // 所有参与组队的卡牌
    std::map<int, std::vector<SupportDeckCard>>& supportCards,        // 全部卡牌（用于计算支援卡组加成）
    const std::function<Score(const DeckDetail &)> &scoreFunc,    
    RecommendCalcInfo& saInfo,
    int limit, 
    bool isChallengeLive, 
    int member, 
    int honorBonus, 
    std::optional<int> eventType, 
    std::optional<int> eventId,
    const std::vector<CardDetail>& fixedCards
)
{   
    // 该算法已停止维护
    throw std::runtime_error("SA algorithm is deprecated, please use DFS/GA algorithm instead");
    
    // // 防止挑战Live卡的数量小于允许上场的数量导致无法组队
    // if (isChallengeLive) {
    //     member = std::min(member, int(cardDetails.size()));
    // }

    // // 实际需要退火的卡牌数量
    // member = member - fixedCards.size();

    // // 根据卡的角色map参与组队的卡牌
    // constexpr int MAX_CID = 27;
    // std::vector<CardDetail> charaCardDetails[MAX_CID] = {};
    // for (const auto& card : cardDetails) 
    //     charaCardDetails[card.characterId].push_back(card);
    
    // double temperature = cfg.saStartTemperature;
    // auto start_time = std::chrono::high_resolution_clock::now();
    // int iter_num = 0;
    // int no_improve_iter_num = 0;
    // double current_score = 0;
    // double last_score = 0;
    // std::vector<int> replacableCardIndices{};
    // std::set<int> deckCharacters{};
    // std::set<int> deckCardIds{};
    
    // // 根据综合力生成一个初始卡组
    // std::vector<const CardDetail*> deck{};
    // if (member > 0)
    // {
    //     if (!isChallengeLive) {
    //         // 遍历所有角色 找到每个角色综合力最高的一个卡牌（不能是和fixedCards相同的角色）
    //         for (int i = 0; i < MAX_CID; ++i) {
    //             auto& cards = charaCardDetails[i];
    //             if (cards.empty()) continue;
    //             if (std::find_if(fixedCards.begin(), fixedCards.end(), [&](const CardDetail& card) {
    //                 return card.characterId == i;
    //             }) != fixedCards.end()) continue;
    //             auto& max_card = *std::max_element(cards.begin(), cards.end(), [](const CardDetail& a, const CardDetail& b) {
    //                 return a.power.min != b.power.min ? a.power.min < b.power.min : a.cardId > b.cardId;
    //             });
    //             deck.push_back(&max_card);
    //         }
    //     } else {
    //         // 选取全部（不能是和fixedCards相同的卡）
    //         for (auto& card : cardDetails) {
    //             if (std::find_if(fixedCards.begin(), fixedCards.end(), [&](const CardDetail& fixedCard) {
    //                 return card.cardId == fixedCard.cardId;
    //             }) != fixedCards.end()) continue;
    //             deck.push_back(&card);
    //         }
    //     }
    //     // 再排序后resize为member数量
    //     std::sort(deck.begin(), deck.end(), [](const CardDetail* a, const CardDetail* b) {
    //         return a->power.min != b->power.min ? a->power.min > b->power.min : a->cardId < b->cardId;
    //     });
    //     if (int(deck.size()) > member) 
    //         deck.resize(member);
    //     // 计算当前综合力
    //     auto recDeck = getBestPermutation(
    //         this->deckCalculator, deck, supportCards, scoreFunc, 
    //         honorBonus, eventType, eventId, liveType, cfg
    //     );
    //     // 记录当前卡组
    //     for (const auto& card : deck) {
    //         deckCharacters.insert(card->characterId);
    //         deckCardIds.insert(card->cardId);
    //     }
    //     saInfo.update(recDeck, limit);
    //     saInfo.deckTargetValueMap[calcDeckHash(deck)] = recDeck.targetValue;
    // }

    // // 添加固定卡牌（在末尾）
    // for (const auto& card : fixedCards) {
    //     deck.push_back(&card);
    //     deckCharacters.insert(card.characterId);
    //     deckCardIds.insert(card.cardId);
    // }

    // // 如果member=0，不需要退火
    // if (member == 0) {
    //     saInfo.update(getBestPermutation(
    //         this->deckCalculator, deck, supportCards, scoreFunc, 
    //         honorBonus, eventType, eventId, liveType, cfg
    //     ), limit);
    //     return;
    // }

    // // 退火
    // while (true) {
    //     // 随机选一个位置替换（不能是固定卡牌）
    //     int pos = std::uniform_int_distribution<int>(0, int(deck.size()) - int(fixedCards.size()) - 1)(rng);

    //     // 收集该位置能够替换的卡的索引 chara_index * 10000 + card_index
    //     replacableCardIndices.clear();
    //     for (int i = 0; i < MAX_CID; ++i) {
    //         // 不是挑战live的情况，排除和其他几个卡角色相同的
    //         if (!isChallengeLive && i != deck[pos]->characterId && deckCharacters.count(i))
    //             continue;
    //         for(int j = 0; j < int(charaCardDetails[i].size()); j++) {
    //             // 如果是挑战live，需要排除和其他卡重复（不是挑战live的情况不用，因为其他卡会被角色相同判断排除）
    //             // 但是不排除需要替换的那张卡，避免出现没有能够替换的问题
    //             if (isChallengeLive && charaCardDetails[i][j].cardId != deck[pos]->cardId 
    //                 && deckCardIds.count(charaCardDetails[i][j].cardId))
    //                 continue;
    //             replacableCardIndices.push_back(i * 10000 + j);
    //         }
    //     }
        
    //     // 随机一个进行替换
    //     int index = std::uniform_int_distribution<int>(0, int(replacableCardIndices.size()) - 1)(rng);
    //     int chara_index = replacableCardIndices[index] / 10000;
    //     int card_index = replacableCardIndices[index] % 10000;
    //     auto old_card = deck[pos];
    //     auto new_card = &charaCardDetails[chara_index][card_index];
        
    //     // 替换，计算新的综合力，并计算接受概率
    //     deck[pos] = new_card;
    //     long long hash = calcDeckHash(deck);
    //     bool visited = saInfo.deckTargetValueMap.count(hash);
    //     double new_score = 0;
    //     if (visited) {
    //         // 如果已经计算过这个组合，直接取值
    //         new_score = saInfo.deckTargetValueMap[hash];
    //     }
    //     else {
    //         auto recDeck = getBestPermutation(
    //             this->deckCalculator, deck, supportCards, scoreFunc, 
    //             honorBonus, eventType, eventId, liveType, cfg
    //         );
    //         new_score = recDeck.targetValue;
    //         saInfo.deckTargetValueMap[hash] = new_score;
    //         // 记录当前卡组答案
    //         saInfo.update(recDeck, limit);
    //     }

    //     double delta = new_score - current_score;
    //     double accept_prob = 0.0;
    //     if (delta > 0) {
    //         accept_prob = 1.0;
    //     } else {
    //         accept_prob = std::exp(delta / temperature);
    //     }

    //     // 以一定概率接受新的卡牌
    //     if (std::uniform_real_distribution<double>(0.0, 1.0)(rng) < accept_prob) {
    //         // 替换
    //         deckCharacters.erase(old_card->characterId);
    //         deckCardIds.erase(old_card->cardId);
    //         deckCharacters.insert(new_card->characterId);
    //         deckCardIds.insert(new_card->cardId);
    //         deck[pos] = new_card;
    //         last_score = current_score;
    //         current_score = new_score;
    //     } else {
    //         // 恢复
    //         deck[pos] = old_card;
    //         last_score = current_score;
    //     }

    //     if (cfg.saDebug) {
    //         std::cerr << "sa iter: " << iter_num << ", score: " << new_score 
    //                 << ", last_score: " << last_score << ", temp: " << temperature 
    //                 << ", prob: " << accept_prob << " no_impro_iter: " << no_improve_iter_num 
    //                 << std::endl;
    //     }

    //     // 超出次数限制
    //     if (++iter_num >= cfg.saMaxIter) 
    //         break;
    //     // 超出未改进次数限制
    //     if (current_score <= last_score) {
    //         if(++no_improve_iter_num >= cfg.saMaxIterNoImprove) 
    //             break;
    //     } else {
    //         no_improve_iter_num = 0;
    //     }
    //     // 超出时间限制
    //     auto current_time = std::chrono::high_resolution_clock::now();
    //     auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
    //     if (elapsed_time > cfg.saMaxTimeMs) 
    //         break;  
    //     // 超出总时间限制
    //     if (saInfo.isTimeout()) 
    //         break;
    //     temperature *= cfg.saCoolingRate;
    // }
}
