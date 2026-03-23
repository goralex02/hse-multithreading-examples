## Кооперативная многозадачность

Обход графа в глубину

Запуск
```
sudo apt update
sudo apt install build-essential libboost-all-dev
g++ -std=c++17 main.cpp -o dfs_boost -lboost_context -lboost_coroutine
./dfs_boost
```

Граф 

0 -> 1, 2
1 -> 3, 4
2 -> 5
4 -> 6

## Тестирование

![hw1](hw1.PNG)
