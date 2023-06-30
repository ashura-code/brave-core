/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <string>
#include <utility>

#include "base/test/mock_callback.h"
#include "base/test/task_environment.h"
#include "brave/components/brave_rewards/core/endpoint/gemini/post_account/post_account_gemini.h"
#include "brave/components/brave_rewards/core/rewards_callbacks.h"
#include "brave/components/brave_rewards/core/rewards_engine_client_mock.h"
#include "brave/components/brave_rewards/core/rewards_engine_impl_mock.h"
#include "net/http/http_status_code.h"
#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=GeminiPostAccountTest.*

using ::testing::_;

namespace brave_rewards::internal {
namespace endpoint {
namespace gemini {

class GeminiPostAccountTest : public testing::Test {
 protected:
  base::test::TaskEnvironment task_environment_;
  MockRewardsEngineImpl mock_engine_impl_;
  PostAccount post_account_{mock_engine_impl_};
};

TEST_F(GeminiPostAccountTest, ServerOK) {
  EXPECT_CALL(*mock_engine_impl_.mock_client(), LoadURL(_, _))
      .Times(1)
      .WillOnce([](mojom::UrlRequestPtr request, auto callback) {
        auto response = mojom::UrlResponse::New();
        response->status_code = net::HTTP_OK;
        response->url = request->url;
        response->body = R"({
              "account": {
                "accountName": "Primary",
                "shortName": "primary",
                "type": "exchange",
                "created": "1619040615242",
                "verificationToken": "mocktoken"
              },
              "users": [{
                "name": "Test",
                "lastSignIn": "2021-04-30T18:46:03.017Z",
                "status": "Active",
                "countryCode": "US",
                "isVerified": true
              }],
              "memo_reference_code": "GEMAPLLV"
            })";
        std::move(callback).Run(std::move(response));
      });

  base::MockCallback<PostAccountCallback> callback;
  EXPECT_CALL(callback, Run(mojom::Result::OK, std::string("mocktoken"),
                            std::string("Test")))
      .Times(1);
  post_account_.Request("4c2b665ca060d912fec5c735c734859a06118cc8",
                        callback.Get());

  task_environment_.RunUntilIdle();
}

TEST_F(GeminiPostAccountTest, ServerError401) {
  EXPECT_CALL(*mock_engine_impl_.mock_client(), LoadURL(_, _))
      .Times(1)
      .WillOnce([](mojom::UrlRequestPtr request, auto callback) {
        auto response = mojom::UrlResponse::New();
        response->status_code = net::HTTP_UNAUTHORIZED;
        response->url = request->url;
        response->body = "";
        std::move(callback).Run(std::move(response));
      });

  base::MockCallback<PostAccountCallback> callback;
  EXPECT_CALL(callback,
              Run(mojom::Result::EXPIRED_TOKEN, std::string(), std::string()))
      .Times(1);
  post_account_.Request("4c2b665ca060d912fec5c735c734859a06118cc8",
                        callback.Get());

  task_environment_.RunUntilIdle();
}

TEST_F(GeminiPostAccountTest, ServerError403) {
  EXPECT_CALL(*mock_engine_impl_.mock_client(), LoadURL(_, _))
      .Times(1)
      .WillOnce([](mojom::UrlRequestPtr request, auto callback) {
        auto response = mojom::UrlResponse::New();
        response->status_code = net::HTTP_FORBIDDEN;
        response->url = request->url;
        response->body = "";
        std::move(callback).Run(std::move(response));
      });

  base::MockCallback<PostAccountCallback> callback;
  EXPECT_CALL(callback,
              Run(mojom::Result::EXPIRED_TOKEN, std::string(), std::string()))
      .Times(1);
  post_account_.Request("4c2b665ca060d912fec5c735c734859a06118cc8",
                        callback.Get());

  task_environment_.RunUntilIdle();
}

TEST_F(GeminiPostAccountTest, ServerError404) {
  EXPECT_CALL(*mock_engine_impl_.mock_client(), LoadURL(_, _))
      .Times(1)
      .WillOnce([](mojom::UrlRequestPtr request, auto callback) {
        auto response = mojom::UrlResponse::New();
        response->status_code = net::HTTP_NOT_FOUND;
        response->url = request->url;
        response->body = "";
        std::move(callback).Run(std::move(response));
      });

  base::MockCallback<PostAccountCallback> callback;
  EXPECT_CALL(callback,
              Run(mojom::Result::NOT_FOUND, std::string(), std::string()))
      .Times(1);
  post_account_.Request("4c2b665ca060d912fec5c735c734859a06118cc8",
                        callback.Get());

  task_environment_.RunUntilIdle();
}

TEST_F(GeminiPostAccountTest, ServerErrorRandom) {
  EXPECT_CALL(*mock_engine_impl_.mock_client(), LoadURL(_, _))
      .Times(1)
      .WillOnce([](mojom::UrlRequestPtr request, auto callback) {
        auto response = mojom::UrlResponse::New();
        response->status_code = 418;
        response->url = request->url;
        response->body = "";
        std::move(callback).Run(std::move(response));
      });

  base::MockCallback<PostAccountCallback> callback;
  EXPECT_CALL(callback,
              Run(mojom::Result::FAILED, std::string(), std::string()))
      .Times(1);
  post_account_.Request("4c2b665ca060d912fec5c735c734859a06118cc8",
                        callback.Get());

  task_environment_.RunUntilIdle();
}

}  // namespace gemini
}  // namespace endpoint
}  // namespace brave_rewards::internal
