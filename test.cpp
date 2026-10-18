#include "logic.hpp"
#include <gtest/gtest.h>

using namespace demo;

TEST(DeliveryTest, LocalOrderHasBaseFee) {
    //Arrange
    DeliveryPolicy policy;
    Shipment shipment{100000};
    //Act
    const auto fee = policy.quote(shipment);
    //Assert
    EXPECT_EQ(fee, 30000);
}
TEST(DeliveryTest, ThresholOrderHasFreeLocalDelivery) {
    //Arrange
    DeliveryPolicy policy;
    //Act + Assert
    EXPECT_EQ(policy.quote(Shipment{500000}), 0);
}

TEST(DeliveryTest, NegativeSubtotalISRejected) {
    //Arrange
    DeliveryPolicy policy;
    //Act + Assert
    EXPECT_THROW(policy.quote(Shipment{-1}), std::invalid_argument);
}

TEST(DeliveryTest, RemotedOrderKeepsFeeAbove) {
    //Arrange
    DeliveryPolicy policy;
    Shipment shipment{500000, 1000, Zone::Remote};
    EXPECT_EQ(policy.quote(shipment), 70000);
}

//Фикстура

class LocalShipmentTest : public ::testing::Test {
protected:
    DeliveryPolicy policy;
    Shipment shipment{100000, 1000, Zone::Local};
    void SetUp() override {shipment.grams = 1000; }
};


TEST_F(LocalShipmentTest, ZeroWeightIsRejected) {
    shipment.grams = 0;
    EXPECT_THROW(policy.quote(shipment), std::invalid_argument);
}

TEST_F(LocalShipmentTest, OverweightIsRejected) {
    shipment.grams = 50001;
    EXPECT_THROW(policy.quote(shipment), std::invalid_argument);
}

struct FeeCase { // тип, которым мы параметризуем тест
    const char* name;
    Shipment shipment;
    Cents expected;
};

class FeeMatrixTest : public ::testing::TestWithParam<FeeCase> {};

TEST_P(FeeMatrixTest, FollowsTariff) {
    const auto& p = GetParam();
    DeliveryPolicy policy;
    EXPECT_EQ(policy.quote(p.shipment), p.expected);
}


const FeeCase fee_cases[] = {
{"LocalBelow", {499999, 1000, Zone::Local}, 30000},
{"LocalAt", {500000, 1000, Zone::Local}, 0},
{"LocalAbove", {500001, 1000, Zone::Local}, 0},
{"RemoteBelow", {499999, 1000, Zone::Remote}, 70000},
{"RemoteAt", {500000, 1000, Zone::Remote}, 70000},
{"RemoteAbove", {500001, 1000, Zone::Remote}, 70000},
//дз1
{"LoyalLocalAt", {300000, 1000, Zone::Local, true}, 0},
{"LoyalLocalBelow",  {299999, 1000, Zone::Local, true},  30000},
{"LoyalLocalAbove",  {300001, 1000, Zone::Local, true},  0},
{"RegularLocalAt", {300000, 1000, Zone::Local, false}, 30000},
{"LoyalRemoteLow",  {100000, 1000, Zone::Remote, true}, 50000},
{"LoyalRemoteHigh", {500000, 1000, Zone::Remote, true}, 50000},
//дз2
{"LocalMass20000", {100000, 20000, Zone::Local},  30000},
{"LocalMass20001", {100000, 20001, Zone::Local}, 130000},
{"LocalMass24999", {100000, 24999, Zone::Local}, 130000},
{"LocalMass25000", {100000, 25000, Zone::Local}, 230000},
{"LocalMass30000", {100000, 30000, Zone::Local}, 330000},
{"LocalMass50000", {100000, 50000, Zone::Local}, 730000},
{"RemoteMass20000", {100000, 20000, Zone::Remote}, 70000},
{"RemoteMass20001", {100000, 20001, Zone::Remote}, 100000},
{"RemoteMass21000", {100000, 21000, Zone::Remote}, 100000},
{"RemoteMass21001", {100000, 21001, Zone::Remote}, 130000},
{"RemoteMass50000", {100000, 50000, Zone::Remote}, 970000},
{"FreeLocalWithOverweight", {500000, 25000, Zone::Local}, 200000},
{"LoyalFreeLocalOverweight", {300000, 20001, Zone::Local, true}, 100000},
{"LoyalRemoteOverweight", {100000, 20001, Zone::Remote, true}, 80000},


};

INSTANTIATE_TEST_SUITE_P(Tariffs, FeeMatrixTest, 
    ::testing::ValuesIn(fee_cases), 
    [](const ::testing::TestParamInfo<FeeCase>& info) {
        return std::string{info.param.name};
    });


TEST_F(LocalShipmentTest, MaxWeightIsAccepted) {
    shipment.grams = 50000;
    EXPECT_NO_THROW(policy.quote(shipment));
}