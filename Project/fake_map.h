#ifndef FAKE_MAP_H
#define FAKE_MAP_H

#include <QList>
#include <QMap>

/* 为了解决QMap自动排序的问题，这里用QList伪造了一个Map类，实现了Map的功能接口，但是本质是List，*/
/* 所以find等操作的时间复杂度是O(n),不要对它的效率有所期待 */
/* 但是这个fake Map只用于有较少选项的combobox使用，所以对软件效率没有影响 */

/* 泛型编程 ，模板声明和定义建议放在同一个文件中，也就是头文件。不然容易出现各种链接器错误 */
template <typename K, typename V>
class fake_map
{
private:
    QList<QPair<K,V>> kernel;

public:
    fake_map(){
        kernel = QList<QPair<K,V>>();
    }
    /**
     * @brief insert 插入key-value对  It will not be sorted !!
     */
    void insert(K key,V value){
        QPair<K,V> pair(key,value);
        kernel.append(pair);
    }
    /**
     * @brief find 假装自己是个Map， 用遍历手段找到key对应的value
     * @return 找到的key对应的point
     */
    typename QList<QPair<K,V>>::iterator find(K key){
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
    typename QList<QPair<K,V>>::iterator begin(){
        return kernel.begin();
    }
    typename QList<QPair<K,V>>::iterator end(){
        return kernel.end();
    }
    /**
     * @brief isEmpty 判断是否为空
     * @return
     */
    bool isEmpty(){
        return kernel.isEmpty();
    }
    /**
     * @brief value 通过key值找到value
     * @return
     */
    V value(K key){
        for(auto pair : kernel){
            if(pair.first == key)
                return pair.second;
        }
        return nullptr;
    }
    /**
     * @brief clear 试图清空
     */
    void clear(){
        kernel.clear();
    }
};

#endif // FAKE_MAP_H
