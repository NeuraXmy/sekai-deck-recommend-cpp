import os

from .sekai_deck_recommend import (
    DeckRecommendUserData,
    DeckRecommendCardConfig,
    DeckRecommendSingleCardConfig,
    DeckRecommendSaOptions,
    DeckRecommendGaOptions,
    DeckRecommendOptions,
    RecommendCard,
    RecommendDeck,
    DeckRecommendResult,
    SekaiDeckRecommend,
)

from . import sekai_deck_recommend as _extension_module

_package_root = os.path.dirname(__file__)
_data_dir = os.path.join(_package_root, "data")
_extension_module.init_data_path(_data_dir)

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