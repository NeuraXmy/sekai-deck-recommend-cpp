from typing import Optional, Dict, Any, List, Union


class DeckRecommendUserData:
    """
    User data for deck recommendation
    Methods:
        load_from_file(path: str) -> None: Load user data from a local file
        load_from_bytes(data: Union[str, bytes]) -> None: Load user data from string or bytes
    """
    
    def __init__(self) -> None:
        ...
    
    def load_from_file(self, path: str) -> None:
        ...
    
    def load_from_bytes(self, data: Union[str, bytes]) -> None:
        ...


class DeckRecommendCardConfig:
    """
    Card config for a specific rarity
    Attributes:
        disable (bool): Disable this rarity, default is False
        level_max (bool): Always use max level, default is False
        episode_read (bool): Always use read episode, default is False
        master_max (bool): Always use max master rank, default is False
        skill_max (bool): Always use max skill level, default is False
        canvas (bool): Always use canvas bonus, default is False
    """
    
    disable: Optional[bool]
    level_max: Optional[bool]
    episode_read: Optional[bool]
    master_max: Optional[bool]
    skill_max: Optional[bool]
    canvas: Optional[bool]

    def to_dict(self) -> Dict[str, Any]:
        ...
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'DeckRecommendCardConfig':
        ...


class DeckRecommendSingleCardConfig:
    """
    Card config for single card
    Attributes:
        card_id (int): Card ID
        disable (bool): Disable this card, default is False
        level_max (bool): Always use max level, default is False
        episode_read (bool): Always use read episode, default is False
        master_max (bool): Always use max master rank, default is False
        skill_max (bool): Always use max skill level, default is False
        canvas (bool): Always use canvas bonus, default is False
    """
    
    card_id: int
    disable: Optional[bool]
    level_max: Optional[bool]
    episode_read: Optional[bool]
    master_max: Optional[bool]
    skill_max: Optional[bool]
    canvas: Optional[bool]

    def to_dict(self) -> Dict[str, Any]:
        ...
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'DeckRecommendSingleCardConfig':
        ...


class DeckRecommendSaOptions:
    """
    Simulated annealing options
    Attributes:
        run_num (int): Number of simulated annealing runs, default is 20
        seed (int): Random seed, leave it None or use -1 for random seed, default is None
        max_iter (int): Maximum iterations, default is 1000000
        max_no_improve_iter (int): Maximum iterations without improvement, default is 10000
        time_limit_ms (int): Time limit of each run in milliseconds, default is 200
        start_temprature (float): Start temperature, default is 1e8
        cooling_rate (float): Cooling rate, default is 0.999
        debug (bool): Whether to print debug information, default is False
    """
    run_num: Optional[int]
    seed: Optional[int]
    max_iter: Optional[int]
    max_no_improve_iter: Optional[int]
    time_limit_ms: Optional[int]
    start_temprature: Optional[float]
    cooling_rate: Optional[float]
    debug: Optional[bool]

    def to_dict(self) -> Dict[str, Any]:
        ...
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'DeckRecommendSaOptions':
        ...


class DeckRecommendGaOptions:
    """
    Genetic algorithm options
    Attributes:
        seed (int): Random seed, leave it None or use -1 for random seed, default is None
        debug (bool): Whether to print debug information, default is False
        max_iter (int): Maximum iterations, default is 1000000
        max_no_improve_iter (int): Maximum iterations without improvement, default is 5
        pop_size (int): Population size, default is 10000
        parent_size (int): Parent size, default is 1000
        elite_size (int): Elite size, default is 0
        crossover_rate (float): Crossover rate, default is 1.0
        base_mutation_rate (float): Base mutation rate, default is 0.1
        no_improve_iter_to_mutation_rate (float): Rate of no improvement iterations to mutation rate (mutation_rate = base_mutation_rate + no_improve_iter * no_improve_iter_to_mutation_rate), default is 0.02
    """
    seed: Optional[int]
    debug: Optional[bool]
    max_iter: Optional[int]
    max_no_improve_iter: Optional[int]
    pop_size: Optional[int]
    parent_size: Optional[int]
    elite_size: Optional[int]
    crossover_rate: Optional[float]
    base_mutation_rate: Optional[float]
    no_improve_iter_to_mutation_rate: Optional[float]

    def to_dict(self) -> Dict[str, Any]:
        ...
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'DeckRecommendGaOptions':
        ...


class DeckRecommendOptions:
    """
    Deck recommend options
    Attributes:
        target (str): Target of the recommendation in ["score", "power", "skill", "bonus"], default is "score"
        algorithm (str): "dfs" for brute force, "sa" for simulated annealing, "ga" for genetic algorithm, default is "ga"
        region (str): Region in ["jp", "en", "tw", "kr", "cn"]
        user_data (DeckRecommendUserData): User suite data for deck recommendation
        user_data_file_path (str): File path of user suite data json
        user_data_str (str | bytes): String or bytes of user suite data json
        live_type (str): Live type in ["multi", "solo", "auto", "challenge", "challenge_auto", "mysekai"]
        music_id (int): Music ID
        music_diff (str): Music difficulty in ["easy", "normal", "hard", "expert", "master", "append"]
        event_id (int): Event ID, leave it None to use no-event or unit-attr-specificed recommendation
        event_attr (str): Attribute of unit-attr-specificed recommendation, only available when event_id is None. In ["mysterious", "cute", "cool", "pure", "happy"]
        event_unit (str): Unit of unit-attr-specificed recommendation, only available when event_id is None. In ["light_sound", "idol", "street", "theme_park", "school_refusal", "piapro"]
        event_type (str): Event type of unit-attr-specificed/no-event recommendation, only available when event_id is None. In ["marathon", "cheerful_carnival"]
        world_bloom_event_turn (int): World bloom event turn, only available when event_id is None, In [1, 2]
        world_bloom_character_id (int): World bloom character ID, only required when event is world bloom
        challenge_live_character_id (int): Challenge live character ID, only required when live is challenge live
        limit (int): Limit of returned decks, default is 10. No guarantee to return this number of decks if not enough cards
        member (int): Number of members in the deck, default is 5
        timeout_ms (int): Timeout in milliseconds, default is None
        rarity_1_config (DeckRecommendCardConfig): Card config for rarity 1
        rarity_2_config (DeckRecommendCardConfig): Card config for rarity 2
        rarity_3_config (DeckRecommendCardConfig): Card config for rarity 3
        rarity_birthday_config (DeckRecommendCardConfig): Card config for birthday cards
        rarity_4_config (DeckRecommendCardConfig): Card config for rarity 4
        single_card_configs (List[DeckRecommendSingleCardConfig]): Card config for single cards that will override rarity configs.
        filter_other_unit (bool): Whether to filter out other units for banner event, default is False
        fixed_cards (List[int]): List of card IDs that always included in the deck, default is None
        fixed_characters (List[int]): List of character IDs that always included in the deck (first is always leader), cannot used in challenge live, cannot used with fixed_cards together, default is None
        target_bonus_list (List[int]): List of target event bonus, required when target is "bonus"
        skill_reference_choose_strategy (str): Strategy for bfes skill reference choose in ["average", "max", "min"], default is "average"
        keep_after_training_state (bool): Whether to keep after-training state of bfes cards, default is False
        multi_live_teammate_score_up (int): Score up of single multi-live teammate, default is None (None means copying self score up)
        multi_live_teammate_power (int): Power of single multi-live teammate, default is None (None means copying self power)
        best_skill_as_leader (bool): Whether to use the best skill card as leader, default is True
        multi_live_score_up_lower_bound (float): Lower bound of multi live score up, only available when live_type is "multi", default is 0
        skill_order_choose_strategy (str): Strategy for skill order choose in ["average", "max", "min", "specific"], default is "average"
        specific_skill_order (List[int]): Specific skill order starting from 0, only required when skill_order_choose_strategy is "specific", default is None
        sa_options (DeckRecommendSaOptions): Simulated annealing options
        ga_options (DeckRecommendGaOptions): Genetic algorithm options
    """
    target: Optional[str]
    algorithm: Optional[str]
    region: str
    user_data: Optional[DeckRecommendUserData]
    user_data_file_path: Optional[str]
    user_data_str: Optional[Union[str, bytes]]
    live_type: str
    music_id: int
    music_diff: str
    event_id: Optional[int]
    event_attr: Optional[str]
    event_unit: Optional[str]
    event_type: Optional[str]
    world_bloom_event_turn: int
    world_bloom_character_id: Optional[int]
    challenge_live_character_id: Optional[int]
    limit: Optional[int]
    member: Optional[int]
    timeout_ms: Optional[int]
    rarity_1_config: Optional[DeckRecommendCardConfig]
    rarity_2_config: Optional[DeckRecommendCardConfig]
    rarity_3_config: Optional[DeckRecommendCardConfig]
    rarity_birthday_config: Optional[DeckRecommendCardConfig]
    rarity_4_config: Optional[DeckRecommendCardConfig]
    single_card_configs: Optional[List[DeckRecommendSingleCardConfig]]
    filter_other_unit: Optional[bool]
    fixed_cards: Optional[List[int]]
    fixed_characters: Optional[List[int]]
    target_bonus_list: Optional[List[int]]
    skill_reference_choose_strategy: Optional[str]
    keep_after_training_state: Optional[bool]
    multi_live_teammate_score_up: Optional[int]
    multi_live_teammate_power: Optional[int]
    best_skill_as_leader: Optional[bool]
    multi_live_score_up_lower_bound: Optional[float]
    skill_order_choose_strategy: Optional[str]
    specific_skill_order: Optional[List[int]]
    sa_options: Optional[DeckRecommendSaOptions]
    ga_options: Optional[DeckRecommendGaOptions]

    def to_dict(self) -> Dict[str, Any]:
        ...
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'DeckRecommendOptions':
        ...


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
        episode1_read (bool): Whether episode 1 is read
        episode2_read (bool): Whether episode 2 is read
        after_training (bool): Whether the card is after special training
        default_image (str): Default image of the card in ["original", "special_training"]
        has_canvas_bonus (bool): Whether the card has canvas bonus
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
    episode1_read: bool
    episode2_read: bool
    after_training: bool
    default_image: str
    has_canvas_bonus: bool

    def to_dict(self) -> Dict[str, Any]:
        ...
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'RecommendCard':
        ...


class RecommendDeck:
    """
    Deck recommendation result
    Attributes:
        score (int): event point or challenge score of the deck
        live_score (int): Live score of the deck
        mysekai_event_point (int): event point of the deck obtained in mysekai
        total_power (int): Total power of the deck
        base_power (int): Base power of the deck
        area_item_bonus_power (int): Area item bonus power of the deck
        character_bonus_power (int): Character bonus power of the deck
        honor_bonus_power (int): Honor bonus power of the deck
        fixture_bonus_power (int): Fixture bonus power of the deck
        gate_bonus_power (int): Gate bonus power of the deck
        event_bonus_rate (float): Event bonus rate of the deck
        support_deck_bonus_rate (float): Support deck bonus rate of the deck
        multi_live_score_up (float): final score up of the deck in multi live
        cards (List[RecommendCard]): List of recommended cards in the deck
    """
    score: int
    live_score: int
    mysekai_event_point: int
    total_power: int
    base_power: int
    area_item_bonus_power: int
    character_bonus_power: int
    honor_bonus_power: int
    fixture_bonus_power: int
    gate_bonus_power: int
    event_bonus_rate: float
    support_deck_bonus_rate: float
    multi_live_score_up: float
    cards: List[RecommendCard]

    def to_dict(self) -> Dict[str, Any]:
        ...
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'RecommendDeck':
        ...


class DeckRecommendResult:
    """
    Deck recommendation result
    Attributes:
        decks (List[RecommendDeck]): List of recommended decks
    """
    decks: List[RecommendDeck]

    def to_dict(self) -> Dict[str, Any]:
        ...
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'DeckRecommendResult':
        ...


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
    options.algorithm = "sa"
    options.region = "jp"
    options.user_data_file_path = "user/data/file/path"
    options.live_type = "multi"
    options.music_id = 74
    options.music_diff = "expert"
    options.event_id = 160
    
    result = sekai_deck_recommend.recommend(options)
    ```

    For more details about the options, please refer docstring of `DeckRecommendOptions` class.
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

    def update_masterdata_from_strings(self, data: Dict[str, Union[str, bytes]], region: str) -> None:
        """
        Update master data of the specific region from dictionary of string or bytes
        Args:
            data (Dict[str, bytes]): Dictionary of master data jsons in string or bytes
                example: data = {
                    "cards": "...",
                    "events": "...",
                }
            region (str): Region in ["jp", "en", "tw", "kr", "cn"]
        """
        ...

    def update_musicmetas(self, file_path: str, region: str) -> None:
        """
        Update music metas of the specific region from a local file
        Args:
            file_path (str): File path of music metas
            region (str): Region in ["jp", "en", "tw", "kr", "cn"]
        """
        ...

    def update_musicmetas_from_string(self, data: Union[str, bytes], region: str) -> None:
        """
        Update music metas of the specific region from string or bytes
        Args:
            data (bytes): String or bytes of music metas json
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
