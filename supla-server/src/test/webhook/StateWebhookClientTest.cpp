/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "StateWebhookClientTest.h"
#include "TrivialHttpFactoryMock.h"
#include "TrivialHttpMock.h"
#include "webhook/statewebhookcredentials.h"

namespace testing {

StateWebhookClientTest::StateWebhookClientTest() {}

void StateWebhookClientTest::SetUp() {
  supla_user::init();
  user = new supla_user(1001);
  user->setUniqueId("dc85740d-cb27-405b-9da3-e8be5c71ae5b", "");

  user->stateWebhookCredentials()->set("ACCESS-TOKEN", "RERESH-TOKEN", 3600,
                                       "https://localhost", "130,140");
  client = new supla_state_webhook_client(user->stateWebhookCredentials());

  client->setHttpConnectionFactory(new TrivialHttpFactoryMock());
}
void StateWebhookClientTest::TearDown() {
  delete client->getHttpConnectionFactory();
  delete client;
  delete user;

  supla_user::user_free();
}

TEST_F(StateWebhookClientTest, sendLightSwitchReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 172\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"LIGHTSWITCH\",\"timestamp\":\"1600097258\",\"state\":{\"on\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendLightSwitchReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 174\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"LIGHTSWITCH\",\"timestamp\":\"1600097258\",\"state\":{\"on\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendLightSwitchReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendPowerSwitchReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 172\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"POWERSWITCH\",\"timestamp\":\"1600097258\",\"state\":{\"on\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendPowerSwitchReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 174\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"POWERSWITCH\",\"timestamp\":\"1600097258\",\"state\":{\"on\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendPowerSwitchReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

} /* namespace testing */