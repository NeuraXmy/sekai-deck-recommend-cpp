# Sekai Deck Recommendation C++

A C++ optimized version of [sekai-calculator](https://github.com/xfl03/sekai-calculator) with Python bindings, providing both the original brute-force search algorithm and a new simulated annealing algorithm.

## Install

### Prerequisites
- CMake ≥ 3.15
- C++17 compatible compiler (GCC/Clang/MSVC)
- Python 3.8+ with development headers

### Steps

```bash
# Clone with submodules
git clone --recursive https://github.com/NeuraXmy/sekai-deck-recommend-cpp.git
cd sekai-deck-recommend-cpp

# Install via pip
pip install -e . -v
```

## Usage

```python
from sekai_deck_recommend import (
    SekaiDeckRecommend, 
    DeckRecommendOptions,
    DeckRecommendCardConfig,
    PySaOptions
)
   
sekai_deck_recommend = SekaiDeckRecommend()

sekai_deck_recommend.update_masterdata("base/dir/of/masterdata", "jp")
sekai_deck_recommend.update_musicmetas("file/path/of/musicmetas.json", "jp")

options = DeckRecommendOptions()

# "sa" for simulated annealing, "dfs" for brute-force search
options.algorithm = "sa"   

options.region = "jp"
options.user_data_file_path = "user/data/file/path.json"
options.live_type = "multi"
options.music_id = 74
options.music_diff = "expert"
options.event_id = 160

result = sekai_deck_recommend.recommend(options)
```

For more details of options, please refer the docstring of `sekai_deck_recommend.DeckRecommendOptions`

## Acknowledgments
- Original implementation by [xfl03/sekai-calculator](https://github.com/xfl03/sekai-calculator)
- JSON parsing by [nlohmann/json](https://github.com/nlohmann/json)
- Python bindings powered by [pybind11](https://github.com/pybind/pybind11)
