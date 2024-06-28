#pragma once

#include <string>
#include <vector>
#include <unordered_map>

class Order
{
  
 public:


 Order(const std::string& ordId, const std::string& secId, const std::string& side, const unsigned int qty, const std::string& user,
       const std::string& company)
   : m_orderId(ordId), m_securityId(secId), m_side(side), m_qty(qty), m_user(user), m_company(company) { }

  std::string orderId() const    { return m_orderId; }
  std::string securityId() const { return m_securityId; }
  std::string side() const       { return m_side; }
  std::string user() const       { return m_user; }
  std::string company() const    { return m_company; }
  unsigned int qty() const       { return m_qty; }
  
 private:
  

  std::string m_orderId;
  std::string m_securityId;
  std::string m_side;
  unsigned int m_qty;
  std::string m_user;
  std::string m_company;
    
};


class OrderCacheInterface
{
    
public:
  
  virtual void addOrder(Order order) = 0; 

  virtual void cancelOrder(const std::string& orderId) = 0; 

  virtual void cancelOrdersForUser(const std::string& user) = 0; 

  virtual void cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty) = 0; 

  virtual unsigned int getMatchingSizeForSecurity(const std::string& securityId) = 0; 

  virtual std::vector<Order> getAllOrders() const = 0;  

};

class OrderCacheInterfaceImpl : public OrderCacheInterface {
  public:
  OrderCacheInterfaceImpl() = default;

  void addOrder(Order order) {
    m_orders.emplace(order.orderId(), order);
  }

  void cancelOrder(const std::string& orderId) {
    m_orders.erase(orderId);
  }

  void cancelOrdersForUser(const std::string& user) {
    for(auto& [key_order, order] : m_orders) {
      if (order.user().compare(user) == 0) {
        m_orders.erase(key_order);
      }
    }
  }
  
  void cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty) {
    for(auto& [key_order, order] : m_orders) {
      if (order.securityId().compare(securityId) == 0 && order.qty() >= minQty) {
        m_orders.erase(key_order);
      }
    }
  }

  unsigned int getMatchingSizeForSecurity(const std::string& securityId) {
    //get orders for same securityId
    std::vector<Order> orders_buy, orders_sell;
    for(const auto& [key_order, order] : m_orders) {
      if (order.securityId().compare(securityId) == 0) {
        if (order.side().compare("Buy") == 0) 
            orders_buy.push_back(order);
        else // "Sell"
            orders_sell.push_back(order);
      }
    }

    unsigned int total_matching_size = 0;
    //Check buy orders first
    for(const auto&buy : orders_buy) {
        unsigned int matching_buy = 0;
        for(const auto &sell : orders_sell) {
            if (buy.company().compare(sell.company()) == 0) {
                continue;
            }
            matching_buy += sell.qty(); 
        }
        if (matching_buy > buy.qty()) {
            total_matching_size += buy.qty();
        }
    }

    //Check sell orders then
    for(const auto&sell : orders_sell) {
        unsigned int matching_sell = 0;
        for(const auto &buy : orders_buy) {
            if (sell.company().compare(buy.company()) == 0) {
                continue;
            }
            matching_sell += buy.qty(); 
        }
        if (matching_sell > sell.qty() && total_matching_size < matching_sell) {
            total_matching_size = sell.qty();
        }
    }

    return total_matching_size;
  }

  std::vector<Order> getAllOrders() const {
    std::vector<Order> orders;
    orders.reserve(m_orders.size());
    for(const auto& [_, order] : m_orders) {
      orders.push_back(order);
    }
    return orders;
  }  

  private:
  std::unordered_map<std::string, Order> m_orders;
};