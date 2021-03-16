#include "fake_map.h"

template <typename K, typename V>
fake_map<K,V>::fake_map(){
    kernel = QList<QPair<K,V>>();
}

/**
 * @brief insert 插入key-value对  It will not be sorted !!
 */
template <typename K, typename V>
void fake_map<K,V>::insert(K key,V value){
    QPair<K,V> pair(key,value);
    kernel.append(pair);
}
/**
 * @brief find 假装自己是个Map， 用遍历手段找到key对应的value
 * @return 找到的key对应的point
 */
template <typename K, typename V>
typename QList<QPair<K,V>>::iterator fake_map<K,V>::find(K key){
    auto itor = kernel.begin();
    while(itor != kernel.end() && itor->first != key){
        ++itor;
    }
    return itor;
}
/**
 * @brief begin end 迭代器相关
 * @return 迭代器
 */
template <typename K, typename V>
typename QList<QPair<K,V>>::iterator fake_map<K,V>::begin(){
    return kernel.begin();
}
template <typename K, typename V>
typename QList<QPair<K,V>>::iterator fake_map<K,V>::end(){
    return kernel.end();
}
/**
 * @brief isEmpty 判断是否为空
 * @return
 */
template <typename K, typename V>
bool fake_map<K,V>::isEmpty(){
    return kernel.isEmpty();
}
/**
 * @brief value 通过key值找到value
 * @return
 */
template <typename K, typename V>
V fake_map<K,V>::value(K key){
    for(auto pair : kernel){
        if(pair.first == key)
            return pair.second;
    }
    return nullptr;
}
