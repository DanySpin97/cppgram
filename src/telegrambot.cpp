#include <cpr/cpr.h>
#include <json/json.h>

#include "cppgram/defines.h"
#include "cppgram/telegrambot.h"
#include "cppgram/exceptions.h"
#include "cppgram/singleton.h"
#include "cppgram/utils.h"
#include "cppgram/structures.h"

using namespace cppgram;
using namespace std;
using namespace util;

TelegramBot::TelegramBot(const string &api_token, const bool &background,
                         const string &filename, const uid_32 &timeout, const uid_32 &limit)
        : bot_token(api_token), updateId(0),
          timeout(timeout), update_limit(limit),
          reader(Singleton::getInstance()->getReader())
{

    if (background) {
        int bg = osutil::backgroundProcess();
        if (bg == OSUTIL_NEWPROC_NOTSUPPORTED) {
            log(Log::Error, "Your operating system is not supported (not yet) for background process");
            throw new BgProcessOSNotSupported;
        } else if (bg == OSUTIL_NEWPROC_FAILED) {
            log(Log::Error, "Error while creating background process");
            throw new BgProcessFailed;
        } else if (bg == OSUTIL_NEWPROC_SUCCESS)
            log(Log::Event, "New background process created!!");
    }
    Singleton::getInstance()->setLogFilename(filename);
}

TelegramBot::~TelegramBot()
{
    delete reader;
}

void TelegramBot::run()
{
    processUpdates();
}

// Ask telegram to send all updates that need to be parsed
void TelegramBot::processUpdates()
{
    //runOnce, needed in order to get the initial update_id offset
    //this will not be executed more than 1 time, after the first update
    do {
        const cpr::Response response = request(cpr::Url{TELEGRAMAPI + bot_token + "/getUpdates"},
                                               cpr::Parameters{{"timeout", to_string(timeout)},
                                                  {"limit", to_string(update_limit)}});


        Json::Value valroot;
        if (checkMethodError(response, valroot) && !valroot["result"].empty()) {
            //sets the initial offset
            updateId = valroot["result"][0]["update_id"].asUInt();
        }

    } while (updateId == 0);

    while (1) {
        const cpr::Response response = request(cpr::Url{TELEGRAMAPI + bot_token + "/getUpdates"},
                                               cpr::Parameters{{"timeout", to_string(timeout)},
                                                  {"limit", to_string(update_limit)},
                                                  {"offset", to_string(updateId)}});

        Json::Value valroot;
        if (checkMethodError(response, valroot) && !valroot["result"].empty()) {
            for (Json::Value &val: valroot["result"]) {
                if (!val["message"].isNull()) {
                    processMessage(message(val["message"]));
                } else if (!val["edited_message"].isNull()) {
                    processEditedMessage(message(val["edited_message"]));
                } else if (!val["inline_query"].isNull()) {
                    processInlineQuery(inlineQuery(val["inline_query"]));
                } else if (!val["choosen_inline_result"].isNull()) {
                    processChosenInlineResult(choosenInlineResult(val["choosen_inline_result"]));
                } else if (!val["callback_query"].isNull()) {
                    processCallbackQuery(callbackQuery(val["callback_query"]));
                }
            }
            updateId += valroot["result"].size();
        }
    }
}

bool TelegramBot::getUpdates(Json::Value &val, const uid_32 &offset, const uid_32 &limit, const uid_32 &timeout)
{
    connection->SetUrl(cpr::Url{TELEGRAMAPI + bot_token + "/getUpdates"});
    connection->SetParameters(cpr::Parameters{{"timeout", to_string(timeout)},
                                              {"limit", to_string(limit)},
                                              {"offset", to_string(offset + 1)}});
    const cpr::Response response = connection->Get();

    if (!checkMethodError(response, val))
        return false;

    return true;
}

bool TelegramBot::editMessageText(const string &inline_message_id,
                                  const string &text,
                                  const string &reply_markup,
                                  const ParseMode &parse_mode,
                                  const bool &disable_web_page_preview) const
{
    string parseMode = "";

    if (parse_mode == ParseMode::HTML)
        parseMode = "HTML";
    else if (parse_mode == ParseMode::Markdown)
        parseMode = "Markdown";

    connection->SetUrl(cpr::Url{TELEGRAMAPI + bot_token + "/editMessageText"});
    connection->SetParameters(cpr::Parameters{{"inline_message_id", inline_message_id},
                                              {"text", text},
                                              {"parse_mode", parseMode},
                                              {"disable_web_page_preview", disable_web_page_preview},
                                              {"reply_markup", reply_markup}});
    const cpr::Response response = connection->Get();

    Json::Value valroot;
    if (!checkMethodError(response, valroot))
        return false;

    return valroot["result"].asBool();
}

bool TelegramBot::editMessageReplyMarkup(const string &inline_message_id,
                                         const string &reply_markup) const
{
    connection->SetUrl(cpr::Url{TELEGRAMAPI + bot_token + "/editMessageReplyMarkup"});
    connection->SetParameters(cpr::Parameters{{"inline_message_id", inline_message_id},
                                              {"reply_markup", reply_markup}});
    const cpr::Response response = connection->Get();

    Json::Value valroot;
    if (!checkMethodError(response, valroot))
        return false;

    return valroot["result"].asBool();
}

bool TelegramBot::editMessageCaption(const string &inline_message_id,
                                     const string &caption,
                                     const string &reply_markup) const
{
    connection->SetUrl(cpr::Url{TELEGRAMAPI + bot_token + "editMessageCaption"});
    connection->SetParameters(cpr::Parameters{{"inline_message_id", inline_message_id},
                                              {"caption", caption},
                                              {"reply_markup", reply_markup}});
    const cpr::Response response = connection->Get();

    Json::Value valroot;
    if (!checkMethodError(response, valroot))
        return false;

    return valroot["result"].asBool();
}

bool TelegramBot::answerInlineQuery(const string &inline_query_id,
                                    const string &results,
                                    const int &cache_time,
                                    const bool &is_personal,
                                    const string &next_offset,
                                    const string &switch_pm_text,
                                    const string &switch_pm_paramter) const
{
    connection->SetUrl(cpr::Url{TELEGRAMAPI + bot_token + "/answerInlineQuery"});
    connection->SetParameters(cpr::Parameters{{"inline_query_id", inline_query_id},
                                              {"results", results},
                                              {"cache_time", to_string(cache_time)},
                                              {"is_personal", is_personal},
                                              {"next_offset", next_offset},
                                              {"switch_pm_text", switch_pm_paramter},
                                              {"switch_pm_parameter", switch_pm_paramter}});
    const cpr::Response response = connection->Get();

    Json::Value valroot;
    if (!checkMethodError(response, valroot))
        return false;

    return valroot["result"].asBool();
}

//virtual functions
void TelegramBot::processMessage(const struct message &message)
{}
void TelegramBot::processEditedMessage(const struct message &editedMessage)
{}
void TelegramBot::processInlineQuery(const struct inlineQuery &inlineQuery)
{}
void TelegramBot::processChosenInlineResult(const struct choosenInlineResult &choosenInlineResult)
{}
void TelegramBot::processCallbackQuery(const struct callbackQuery &callbackQuery)
{}
//
