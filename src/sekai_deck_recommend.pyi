from typing import Optional, Dict, Any, List, Union


class DeckRecommendCardConfig:
    """
    Card config for a specific rarity
    Attributes:
        disable (bool): Disable this rarity, default is False
        level_max (bool): Always use max level, default is False
        episode_read (bool): Always use read episode, default is False
        master_max (bool): Always use max master rank, default is False
        skill_max (bool): Always use max skill level, default is False
    """
    
    disable: Optional[bool]
    level_max: Optional[bool]
    episode_read: Optional[bool]
    master_max: Optional[bool]
    skill_max: Optional[bool]


class DeckRecommendOptions:
    """
    Deck recommend options
    Attributes:
        region (str): Region in ["jp", "en", "tw", "kr", "cn"]
        user_data_file_path (str): File path of user suite data
        live_type (str): Live type in ["multi", "solo", "auto", "challenge"]
        music_id (int): Music ID
        music_diff (str): Music difficulty in ["easy", "normal", "hard", "expert", "master", "append"]
        event_id (int): Event ID, only required when live_type is not "challenge"
        world_bloom_character_id (int): World bloom character ID, only required when event is world bloom
        challenge_live_character_id (int): Challenge live character ID, only required when live_type is "challenge"
        limit (int): Limit of returned decks, default is 10. No guarantee to return this number of decks if not enough cards
        member (int): Number of members in the deck, default is 5
        rarity_1_config (DeckRecommendCardConfig): Card config for rarity 1
        rarity_2_config (DeckRecommendCardConfig): Card config for rarity 2
        rarity_3_config (DeckRecommendCardConfig): Card config for rarity 3
        rarity_birthday_config (DeckRecommendCardConfig): Card config for birthday cards
        rarity_4_config (DeckRecommendCardConfig): Card config for rarity 4
    """
    region: str
    user_data_file_path: str
    live_type: str
    music_id: int
    music_diff: str
    event_id: Optional[int]
    world_bloom_character_id: Optional[int]
    challenge_live_character_id: Optional[int]
    limit: Optional[int]
    member: Optional[int]
    rarity_1_config: Optional[DeckRecommendCardConfig]
    rarity_2_config: Optional[DeckRecommendCardConfig]
    rarity_3_config: Optional[DeckRecommendCardConfig]
    rarity_birthday_config: Optional[DeckRecommendCardConfig]
    rarity_4_config: Optional[DeckRecommendCardConfig]


class RecommendCard:
    """
    Card recommendation result
    Attributes:
        card_id (int): Card ID
        total_power (int): Total power of the card
        base_power (int): Base power of the card
        event_bonus_rate (float): Event bonus rate of the card
        master_rank (int): Master rank of the card
        level (int): Level of the card
        skill_level (int): Skill level of the card
        skill_score_up (int): Skill score up of the card
        skill_life_recovery (int): Skill life recovery of the card
    """
    card_id: int
    total_power: int
    base_power: int
    event_bonus_rate: float
    master_rank: int
    level: int
    skill_level: int
    skill_score_up: int
    skill_life_recovery: int


class RecommendDeck:
    """
    Deck recommendation result
    Attributes:
        score (int): Score of the deck
        total_power (int): Total power of the deck
        base_power (int): Base power of the deck
        area_item_bonus_power (int): Area item bonus power of the deck
        character_bonus_power (int): Character bonus power of the deck
        honor_bonus_power (int): Honor bonus power of the deck
        fixture_bonus_power (int): Fixture bonus power of the deck
        gate_bonus_power (int): Gate bonus power of the deck
        event_bonus_rate (float): Event bonus rate of the deck
        support_deck_bonus_rate (float): Support deck bonus rate of the deck
        cards (List[RecommendCard]): List of recommended cards in the deck
    """
    score: int
    total_power: int
    base_power: int
    area_item_bonus_power: int
    character_bonus_power: int
    honor_bonus_power: int
    fixture_bonus_power: int
    gate_bonus_power: int
    event_bonus_rate: float
    support_deck_bonus_rate: float
    cards: List[RecommendCard]


class DeckRecommendResult:
    """
    Deck recommendation result
    Attributes:
        decks (List[RecommendDeck]): List of recommended decks
    """
    decks: List[RecommendDeck]


class SekaiDeckRecommend:
    """
    Class for event or challenge live deck recommendation  

    Example usage:
    ```
    from sekai_deck_recommend import SekaiDeckRecommend, DeckRecommendOptions
   
    sekai_deck_recommend = SekaiDeckRecommend()

    sekai_deck_recommend.update_masterdata("base/dir/of/masterdata", "jp")
    sekai_deck_recommend.update_musicmetas("file/path/of/musicmetas", "jp")

    options = DeckRecommendOptions()
    options.region = "jp"
    options.user_data_file_path = "user/data/file/path"
    options.live_type = "multi"
    options.music_id = 74
    options.music_diff = "expert"
    options.event_id = 160
    
    result = sekai_deck_recommend.recommend(options)
    ```
    """

    def __init__(self) -> None:
        ...

    def update_masterdata(self, base_dir: str, region: str) -> None:
        """
        Update master data of the specific region from a local directory
        Args:
            base_dir (str): Base directory of master data
            region (str): Region in ["jp", "en", "tw", "kr", "cn"]
        """
        ...

    def update_musicmetas(self, file_path: str, region: str) -> None:
        """
        Update music metas of the specific region  from a local file
        Args:
            file_path (str): File path of music metas
            region (str): Region in ["jp", "en", "tw", "kr", "cn"]
        """
        ...

    def recommend(self, options: DeckRecommendOptions) -> DeckRecommendResult:
        """
        Recommend event or challenge live decks
        Args:
            options (DeckRecommendOptions): Options for deck recommendation
        Returns:
            DeckRecommendResult: Recommended decks sorted by score descending
        """
        ...