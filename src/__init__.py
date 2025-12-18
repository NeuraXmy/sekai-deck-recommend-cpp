import os
import warnings
from . import sekai_deck_recommend as _extension_module

_package_root = os.path.dirname(__file__)
_data_dir = os.path.join(_package_root, "data")
_extension_module.init_data_path(_data_dir)

DeckRecommendUserData = _extension_module.DeckRecommendUserData
DeckRecommendCardConfig = _extension_module.DeckRecommendCardConfig
DeckRecommendSingleCardConfig = _extension_module.DeckRecommendSingleCardConfig
DeckRecommendSaOptions = _extension_module.DeckRecommendSaOptions
DeckRecommendGaOptions = _extension_module.DeckRecommendGaOptions
DeckRecommendOptions = _extension_module.DeckRecommendOptions
RecommendCard = _extension_module.RecommendCard
RecommendDeck = _extension_module.RecommendDeck
DeckRecommendResult = _extension_module.DeckRecommendResult
SekaiDeckRecommend = _extension_module.SekaiDeckRecommend

__all__ = [
    "DeckRecommendUserData",
    "DeckRecommendCardConfig",
    "DeckRecommendSingleCardConfig",
    "DeckRecommendSaOptions",
    "DeckRecommendGaOptions",
    "DeckRecommendOptions",
    "RecommendCard",
    "RecommendDeck",
    "DeckRecommendResult",
    "SekaiDeckRecommend",
]