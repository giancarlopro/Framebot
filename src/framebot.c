/**
 * MIT License
 * Copyright (c) 2016 - 2018 Giancarlo Rocha & Ródgger Bruno
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <framebot/framebot.h>

static bool notification = 0;
static int32_t mode = 2;

char *parse_mode[3] =
{
"Markdown", /* macro MODE_MARKDOWN */
"HTML", /* macro MODE_HTML */
"" /* macro MODE_NULL */
};

/**
 * Authentic bot token
 */
Bot * framebot_init (const char *token) {
    static bool net = 0;
	Bot *obot = bot(token, NULL);

    if(net == 0){
        network_init();
        net = 1;
    }

    User *bot_user = get_me(obot);

    if (bot_user) {
        obot->user = bot_user;

        return obot;
    }

    bot_free(obot);

    return NULL;
}



/**
 * Returns a User object of the owner bot.
 * https://core.telegram.org/bots/api#getme
 */ 
User *get_me (Bot *bot) {
    User * user = NULL;
    refjson *s_json = NULL;

    s_json = generic_method_call(bot->token, API_GETME);
    if(!s_json)
        return NULL;

    user = user_parse(s_json->content);

    close_json(s_json);

    return user;
}



/**
 * Returns the updates list
 * https://core.telegram.org/bots/api#getupdates
 */ 
Framebot *get_updates (Bot *bot, Framebot *framebot, int64_t offset, int32_t limit,
                     int64_t timeout, char *allowed_updates) {

    refjson *s_json = NULL;
    Update *up = NULL;

    s_json = generic_method_call(bot->token, API_GETUPDATES,
        offset, LIMIT_UPDATE(limit), timeout, CONVERT_URL_STRING(allowed_updates));

    if(s_json == NULL){
        return framebot;
    }

    if( !framebot ){
        framebot = calloc(1, sizeof( Framebot ));
    }

    size_t length, i;
    length = json_array_size(s_json->content);
    if(length > 0){

        for (i = 0; i < length; i++) {
            up = update_parse(json_array_get(s_json->content, i));
            framebot_add( framebot, up );
        }
    }

    close_json(s_json);

    return framebot;
}


/**
 * Sends the given message to the given chat.
 * TODO:
 *  - Change the type of 'chat_id'
 * https://core.telegram.org/bots/api#sendmessage
 */
Message * send_message (Bot *bot, char * chat_id, char *text,
            bool disable_web_page_preview, int64_t reply_to_message_id,
            char * reply_markup) {
    Message * message;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_SENDMESSAGE,
        chat_id, text, parse_mode[get_parse_mode()],
        CONVERT_URL_BOOLEAN(disable_web_page_preview),
        CONVERT_URL_BOOLEAN(get_notification()),
        reply_to_message_id, CONVERT_URL_STRING(reply_markup));

    if(!s_json)
        return NULL;

    message = message_parse(s_json->content);

    close_json ( s_json );

    return message;
}

Message * send_message_chat (Bot *bot, int64_t chat_id, char *text,
            bool disable_web_page_preview, int64_t reply_to_message_id,
            char * reply_markup) {
    Message * message;
    char * cchat_id;

    cchat_id = api_ltoa(chat_id);


    message =  send_message(bot, cchat_id, text,
        disable_web_page_preview, reply_to_message_id,
        reply_markup);

    ffree(cchat_id);

    return message;
}



/**
 * Returns the Chat object of the given chat_id
 * https://core.telegram.org/bots/api#getchat
 */ 
Chat *get_chat (Bot *bot, char *chat_id) {
    Chat * chat;
    refjson *s_json = generic_method_call(bot->token, API_getChat, chat_id);
 
    if(!s_json)
        return NULL;

    chat = chat_parse(s_json->content);

    close_json(s_json);

    return chat;
}


Chat * get_chat_chat(Bot *bot, int64_t chat_id){
    Chat * chat;
    char * cchat_id;

    cchat_id = api_ltoa(chat_id);

    chat = get_chat(bot, cchat_id);

    ffree(cchat_id);

    return chat;
}


/**
 * Changes the title of the given chat_id
 * Returns 1 in success, 0 otherwise
 * https://core.telegram.org/bots/api#setchattitle
 */
int set_chat_title (Bot *bot, char *chat_id, char *title) {
    int result;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_setChatTitle,
        chat_id, title);

    if(!s_json)
        return -1;

    result = json_is_true(s_json->content);

    close_json(s_json);

    return result;
}


int set_chat_title_chat (Bot *bot, int64_t chat_id, char *title) {
    int result;
    char *cchat_id;

    cchat_id = api_ltoa(chat_id);

    result = set_chat_title (bot, cchat_id, title);

    ffree(cchat_id);

    return result;
}


/**
 * Returns the requested ChatMember object.
 * https://core.telegram.org/bots/api#getchatmember
 */
ChatMember *get_chat_member (Bot *bot, char *chat_id, int64_t user_id) {
    refjson *s_json;
    ChatMember * chat_member;

    s_json = generic_method_call(bot->token, API_getChatMember,
        chat_id, user_id);

    if(!s_json)
        return NULL;

    chat_member = chat_member_parse(s_json->content);

    close_json(s_json);

    return chat_member;
}


ChatMember *get_chat_member_chat (Bot *bot, int64_t chat_id, int64_t user_id) {
    ChatMember * chat_member;
    char * cchat_id;

    cchat_id = api_ltoa(chat_id);

    chat_member = get_chat_member (bot, cchat_id, user_id);

    ffree(cchat_id);

    return chat_member;
}


/**
 * Changes the given chat or channel description
 * https://core.telegram.org/bots/api#setchatdescription
 */
bool set_chat_description (Bot *bot, char *chat_id, char *description) {
    refjson *s_json;
    bool result;

    s_json = generic_method_call(bot->token, API_setChatDescription,
        chat_id, description);

    if(!s_json)
        return -1;

    result = json_is_true(s_json->content);

    close_json(s_json);

    return result;
}



bool set_chat_description_chat (Bot *bot, int64_t chat_id, char *description) {
    bool result;
    char * cchat_id;

    cchat_id = api_ltoa(chat_id);

    result = set_chat_description (bot, cchat_id, description);

    ffree(cchat_id);

    return result;
}



/**
 * Returns the number of members in the given chat
 *https://core.telegram.org/bots/api#getchatmemberscount
 */ 
uint32_t get_chat_members_count (Bot *bot, char *chat_id) {
    int result;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_getChatMemberCount, chat_id);
    
    if(!s_json)
        return -1;

    result = (int)json_integer_value(s_json->content);

    close_json(s_json);

    return result;
}



uint32_t get_chat_members_count_chat (Bot *bot, int64_t chat_id) {
    int result;
    char * cchat_id;

    cchat_id = api_ltoa(chat_id);

    result = get_chat_members_count (bot, cchat_id);

    ffree(cchat_id);

    return result;
}


/**
 * Ban a chat user
 * https://core.telegram.org/bots/api#kickchatmember
 */
bool kick_chat_member (Bot *bot, char *chat_id, int64_t user_id, int64_t until_date) {
    bool result;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_kickChatMember, chat_id,
        user_id, until_date);

    if(!s_json)
        return -1;

    result = json_is_true(s_json->content);

    close_json(s_json);

    return result;
}

bool kick_chat_member_chat (Bot *bot, int64_t chat_id, int64_t user_id, int64_t until_date) {
    bool result;
    char * cchat_id;

    cchat_id = api_ltoa(chat_id);

    result = kick_chat_member (bot, cchat_id, user_id, until_date);

    ffree(cchat_id);

    return result;
}

/**
 * restrictChatMember
 * Use this method to restrict a user in a supergroup. 
 * The bot must be an administrator in the supergroup
 * for this to work and must have the appropriate admin rights. 
 * Pass True for all boolean parameters to lift restrictions from a user. 
 * Returns True on success.
 * https://core.telegram.org/bots/api#restrictchatmember
 */
bool restrict_chat_member (Bot *bot, char *chat_id, int64_t user_id, int64_t until_date,
            bool can_send_messages, bool can_send_media_messages, bool can_send_other_messages,
            bool can_add_web_page_previews) {

    int result;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_restrictChatMember,
        chat_id, user_id, until_date,
        CONVERT_URL_BOOLEAN(can_send_messages),
        CONVERT_URL_BOOLEAN(can_send_media_messages),
        CONVERT_URL_BOOLEAN(can_send_other_messages),
        CONVERT_URL_BOOLEAN(can_add_web_page_previews) );

    if(!s_json)
        return -1;

    result = json_is_true(s_json->content);

    close_json(s_json);

    return result;
}


bool restrict_chat_member_chat (Bot *bot, int64_t chat_id, int64_t user_id, int64_t until_date,
            bool can_send_messages, bool can_send_media_messages, bool can_send_other_messages,
            bool can_add_web_page_previews) {
    bool result;
    char * cchat_id;

    cchat_id = api_ltoa(chat_id);

    result = restrict_chat_member (bot, cchat_id, user_id, until_date,
            can_send_messages, can_send_media_messages, can_send_other_messages,
            can_add_web_page_previews);

    ffree(cchat_id);

    return result;
}



/**
 * unbanChatMember
 * Use this method to unban a previously kicked user in a supergroup or channel. 
 * The user will not return to the group or channel automatically, 
 * but will be able to join via link, etc. 
 * The bot must be an administrator for this to work. 
 * Returns True on success.
 * https://core.telegram.org/bots/api#unbanchatmember
 */
bool unban_chat_member (Bot *bot, char *chat_id, int64_t user_id) {
    int result;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_unbanChatMember,
        chat_id, user_id);

    if(!s_json)
        return -1;

    result = json_is_true(s_json->content);

    close_json(s_json);

    return result;
}



bool unban_chat_member_chat (Bot *bot, int64_t chat_id, int64_t user_id) {
    bool result;
    char * cchat_id;

    cchat_id = api_ltoa(chat_id);

    result = unban_chat_member (bot, cchat_id, user_id);

    ffree(cchat_id);

    return result;
}


/**
 * leaveChat
 * Use this method for your bot to leave a group, supergroup or channel. Returns True on success.
 * https://core.telegram.org/bots/api#leavechat
 */
bool leave_chat (Bot *bot, char *chat_id) {
    int result;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_leaveChat, chat_id);

    if(!s_json)
        return -1;

    result = json_is_true(s_json->content);

    close_json(s_json);

    return result;
}



bool leave_chat_chat (Bot *bot, int64_t chat_id) {
    bool result;
    char * cchat_id;

    cchat_id = api_ltoa(chat_id);

    result = leave_chat (bot, cchat_id);

    ffree(cchat_id);

    return result;
}




/**
 * promoteChatMember
 * Use this method to promote or demote a user in a supergroup or a channel.
 * The bot must be an administrator in the chat for this to work
 * and must have the appropriate admin rights.
 * Pass False for all boolean parameters to demote a user.
 * Returns True on success.
 * https://core.telegram.org/bots/api#promotechatmember
 */
bool promote_chat_member (Bot *bot, char *chat_id, int64_t user_id, bool can_change_info,
            bool can_post_messages, bool can_edit_messages, bool can_delete_messages,
            bool can_invite_users, bool can_restrict_members, bool can_pin_messages,
            bool can_promote_members) {
    int result;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_promoteChatMember,
        chat_id, user_id, CONVERT_URL_BOOLEAN(can_change_info),
        CONVERT_URL_BOOLEAN(can_post_messages),
        CONVERT_URL_BOOLEAN(can_edit_messages),
        CONVERT_URL_BOOLEAN(can_delete_messages),
        CONVERT_URL_BOOLEAN(can_invite_users),
        CONVERT_URL_BOOLEAN(can_restrict_members),
        CONVERT_URL_BOOLEAN(can_pin_messages),
        CONVERT_URL_BOOLEAN(can_promote_members));

    if(!s_json)
        return -1;

    result = json_is_true(s_json->content);

    close_json(s_json);

    return result;
}


bool promote_chat_member_chat (Bot *bot, int64_t chat_id, int64_t user_id, bool can_change_info,
            bool can_post_messages, bool can_edit_messages, bool can_delete_messages,
            bool can_invite_users, bool can_restrict_members, bool can_pin_messages,
            bool can_promote_members) {
    bool result;
    char * cchat_id;

    cchat_id = api_ltoa(chat_id);

    result = promote_chat_member(bot, cchat_id, user_id, can_change_info,
            can_post_messages, can_edit_messages, can_delete_messages,
            can_invite_users, can_restrict_members, can_pin_messages,
            can_promote_members);

    ffree(cchat_id);

    return result;
}


/**
 * exportChatInviteLink
 * Use this method to export an invite link to a supergroup or a channel.
 * The bot must be an administrator in the chat for this to work
 * and must have the appropriate admin rights.
 * Returns exported invite link as String on success.
 * 
 * You must release the returned string
 * https://core.telegram.org/bots/api#exportchatinvitelink
 */
char *export_chat_invite_link (Bot *bot, char *chat_id) {

    refjson *s_json;
    char * invite_link;

    s_json = generic_method_call(bot->token, API_exportChatInviteLink);

    if(!s_json)
        return NULL;

    invite_link = alloc_string(json_string_value(s_json->content));

    close_json(s_json);

    return invite_link;
}


char *export_chat_invite_link_chat (Bot *bot, int64_t chat_id) {
    char * invite_link, *cchat_id;

    cchat_id = api_ltoa(chat_id);

    invite_link = export_chat_invite_link(bot, cchat_id);

    ffree(cchat_id);

    return invite_link;
}

/**
 * setChatPhoto
 * https://core.telegram.org/bots/api#setchatphoto
 */
int set_chat_photo(Bot *bot, char * chat_id, char *filename){
    bool result;

    IFile ifile;

    ifile.type = SETCHATPHOTO;
    
    /* Unique identifier for the target */
    ifile.chatphoto.chat_id = chat_id;

    ifile.chatphoto.filename = filename;

    MemStore * input;
    refjson *s_json;

    input = call_method_upload(bot->token, ifile);
    if(!input)
        return 0;

    s_json = start_json(input->content);
    mem_store_free(input);

    if(!s_json)
        return -1;

    result = json_is_true(s_json->content);

    close_json(s_json);

    return result;
}

int set_chat_photo_chat(Bot *bot, int64_t chat_id, char *filename){
    bool result;
    char * cchat_id;

    cchat_id = api_ltoa(chat_id);

    result = set_chat_photo(bot, cchat_id, filename);

    ffree(cchat_id);

    return result;
}


/**
 * deleteChatPhoto
 * https://core.telegram.org/bots/api#deletechatphoto
 */
int delete_chat_photo(Bot *bot, char *chat_id){
    refjson *s_json;
    bool btrue;

    s_json = generic_method_call(bot->token, API_deleteChatPhoto,
        chat_id);

    if(!s_json)
        return -1;

    btrue = json_is_true(s_json->content);

    close_json(s_json);

    return btrue;
}

int delete_chat_photo_chat(Bot *bot, int64_t chat_id){
    bool result;
    char * cchat_id;

    cchat_id = api_ltoa(chat_id);

    result = delete_chat_photo(bot, cchat_id);

    ffree(cchat_id);

    return result;
}

/**
 * getChatAdministrators
 * Use this method to get a list of administrators in a chat.
 * On success, returns an Array of ChatMember objects
 * that contains information about all chat administrators except other bots.
 * If the chat is a group or a supergroup and no administrators were appointed,
 * only the creator will be returned.
 * https://core.telegram.org/bots/api#getchatadministrators
 */
ChatMember *get_chat_administrators (Bot *bot, char *chat_id) {
    ChatMember * chat_member_adm;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_getChatAdministrators, chat_id);

    if(!s_json)
        return NULL;

    chat_member_adm = chat_member_array_parse(s_json->content);

    close_json(s_json);

    return chat_member_adm;
}

ChatMember *get_chat_administrators_chat(Bot *bot, int64_t chat_id){
    ChatMember *chat_member_adm;
    char *cchat_id;

    cchat_id = api_ltoa(chat_id);

    chat_member_adm = get_chat_administrators(bot, cchat_id);

    ffree(cchat_id);

    return chat_member_adm;
}

/**
 * https://core.telegram.org/bots/api#pinchatmessage
 */
bool pin_chat_message (Bot *bot, char *chat_id, int64_t message_id) {
    int result;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_pinChatMessage, chat_id, message_id,
        CONVERT_URL_BOOLEAN(get_notification()));

    if(!s_json)
        return -1;

    result = json_is_true(s_json->content);

    close_json(s_json);

    return result;
}

bool pin_chat_message_chat(Bot *bot, int64_t chat_id, int64_t message_id) {
    bool result;
    char * cchat_id;

    cchat_id = api_ltoa(chat_id);

    result = pin_chat_message(bot, cchat_id, message_id);

    ffree(cchat_id);

    return result;
}


/**
 * unpinChatMessage
 * https://core.telegram.org/bots/api#unpinchatmessage
 */
bool unpin_chat_message(Bot *bot, char *chat_id){
    int result;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_unpinChatMessage,
            chat_id);

    if(!s_json)
        return -1;

    result = json_is_true(s_json->content);

    close_json(s_json);

    return result;
}

bool unpin_chat_message_chat(Bot *bot, int64_t chat_id){
    bool result;
    char * cchat_id;

    cchat_id = api_ltoa(chat_id);

    result = unpin_chat_message(bot, cchat_id);

    ffree(cchat_id);

    return result;
}

/**
 * Generic method to handle Telegram API Methods responses
 * TODO:
 *  - Error filtering
 */
refjson *generic_method_call (const char *token, char *formats, ...) {
    refjson *s_json = NULL;
    va_list params;
    va_start(params, formats);

    char *method_base = vsformat(formats, params);
    MemStore *response = call_method(token, method_base);
    ffree(method_base);

    if(response){
        s_json = start_json(response->content);
        mem_store_free(response);

        return s_json;
    }

    return NULL;
}



/**
 * getFile
 * https://core.telegram.org/bots/api#getfile
 * info file
 * https://core.telegram.org/bots/api#getfile
 */
File * get_file (Bot * bot, const char * file_id){

    refjson *s_json;

    s_json = generic_method_call(bot->token, API_getfile, file_id);

    if(!s_json)
        return 0;

    File * ofile = file_parse(s_json->content);

    close_json(s_json);

    return ofile;
}

int file_download(Bot * bot, File * ofile, char *dir){
    return call_method_download(bot->token, dir, ofile);
}

/**
 * getUserProfilePhotos
 * https://core.telegram.org/bots/api#getuserprofilephotos
 */
UserProfilePhotos * get_user_profile_photos(Bot * bot, char *user_id,
            int64_t offset, int64_t limit) {
    UserProfilePhotos * oupp;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_getUserProfilePhotos,
        user_id, offset, limit);

    if(!s_json)
        return NULL;

    oupp = user_profile_photos_parse(s_json->content);

    close_json(s_json);

    return oupp;
}

UserProfilePhotos * get_user_profile_photos_chat (Bot * bot, int64_t user_id,
            int64_t offset, int64_t limit) {
    UserProfilePhotos * oupp;
    char *cuser_id;

    cuser_id = api_ltoa(user_id);

    oupp = get_user_profile_photos(bot, cuser_id, offset, limit);

    ffree(cuser_id);

    return oupp;
}

/**
 * sendPhoto
 * https://core.telegram.org/bots/api#sendphoto
 */
Message * send_photo(Bot * bot, char * chat_id, char * filename, char * caption,
                             int64_t reply_to_message_id, char * reply_markup){
    Message * message;

    IFile ifile;

    ifile.type = SENDPHOTO;

    /* Unique identifier for the target */
    ifile.photo.chat_id = chat_id;

    /* Photo to send */
    ifile.photo.filename = filename;

    /* Photo caption (may also be used when resending 
     * photos by file_id), 0-200 characters */
    ifile.photo.caption = caption;

    ifile.photo.parse_mode = parse_mode[get_parse_mode()];

    /* Sends the message silently */
    ifile.photo.disable_notification = CONVERT_URL_BOOLEAN(get_notification());

    /* If the message is a reply, ID of the original message */
    ifile.photo.reply_to_message_id = IFILE_LONG(reply_to_message_id);

    ifile.photo.reply_markup = CONVERT_URL_STRING(reply_markup);

    MemStore * input;
    refjson *s_json;

    input = call_method_upload(bot->token, ifile);
    ffree(ifile.photo.reply_to_message_id);
    if(!input){
        return NULL;
    }

    s_json = start_json(input->content);
    mem_store_free(input);
    if(!s_json)
        return NULL;

    message = message_parse(s_json->content);

    close_json(s_json);

    return message;
}



Message * send_photo_chat(Bot * bot, int64_t chat_id, char * filename, char * caption,
        int64_t reply_to_message_id, char * reply_markup){

    Message * message;
    char * cchat_id;
    
    cchat_id = api_ltoa(chat_id);

    message = send_photo(bot, cchat_id, filename, caption,
                                 reply_to_message_id, reply_markup);

    ffree(cchat_id);

    return message;
}


/**
 * sendAudio
 * https://core.telegram.org/bots/api#sendaudio
 */
Message * send_audio(Bot *bot, char * chat_id, char * filename, char * caption,
            int32_t duration, char * performer, char * title,
            int64_t reply_to_message_id, char * reply_markup){
    
    Message * message;

    IFile ifile;

    ifile.type = SENDAUDIO;

    /* Unique identifier for the target */
    ifile.audio.chat_id = chat_id;

    /* Audio file to send */
    ifile.audio.filename = filename;

    /* Audio caption, 0-200 characters */
    ifile.audio.caption = caption;

    ifile.audio.caption = parse_mode[get_parse_mode()];

    /* Duration of the audio in seconds */
    ifile.audio.duration = IFILE_INT(duration);

    /* Performer */
    ifile.audio.performer = performer;

    /* Track name */
    ifile.audio.title = title;

    /* Sends the message silently */
    ifile.audio.disable_notification = CONVERT_URL_BOOLEAN(get_notification());

    /* If the message is a reply, ID of the original message */
    ifile.audio.reply_to_message_id = IFILE_LONG(reply_to_message_id);

    ifile.audio.reply_markup = CONVERT_URL_STRING(reply_markup);

    MemStore * input;
    refjson *s_json;

    input = call_method_upload(bot->token, ifile);
    ffree(ifile.audio.duration);
    ffree(ifile.audio.reply_to_message_id);
    if(!input)
        return NULL;

    s_json = start_json(input->content);
    mem_store_free(input);
    if(!s_json)
        return NULL;

    message = message_parse(s_json->content);

    close_json(s_json);

    return message;
}



Message * send_audio_chat(Bot * bot, int64_t chat_id, char * filename, char * caption,
            int32_t duration, char * performer, char * title,
            int64_t reply_to_message_id, char * reply_markup){

    Message * message;
    char * cchat_id;
    
    cchat_id = api_ltoa(chat_id);

    message = send_audio(bot, cchat_id, filename, caption, duration,
                                 performer, title, reply_to_message_id, reply_markup);

    ffree(cchat_id);

    return message;
}


/**
 * sendDocument
 * https://core.telegram.org/bots/api#senddocument
 */
Message * send_document(Bot * bot, char * chat_id, char * filename, char * caption,
            int64_t reply_to_message_id, char * reply_markup){

    Message * message;

    IFile ifile;

    ifile.type = SENDDOCUMENT;

    /* Unique identifier for the target */
    ifile.document.chat_id = chat_id;

    /* Document to send */
    ifile.document.filename = filename;

    /* Document caption (may also be used when resending 
     * Documents by file_id), 0-200 characters */
    ifile.document.caption = caption;

    ifile.document.parse_mode = parse_mode[get_parse_mode()];

    /* Sends the message silently */
    ifile.document.disable_notification = CONVERT_URL_BOOLEAN(get_notification());

    /* If the message is a reply, ID of the original message */
    ifile.document.reply_to_message_id = IFILE_LONG(reply_to_message_id);

    ifile.document.reply_markup = CONVERT_URL_STRING(reply_markup);

    MemStore * input;
    refjson *s_json;

    input = call_method_upload(bot->token, ifile);
    ffree(ifile.document.reply_to_message_id);
    if(!input)
        return NULL;

    s_json = start_json(input->content);
    mem_store_free(input);
    if(!s_json)
        return NULL;

    message = message_parse(s_json->content);

    return message;
}




Message * send_document_chat(Bot * bot, int64_t chat_id, char * filename, char * caption,
            int64_t reply_to_message_id, char * reply_markup){

    Message * message;
    char * cchat_id;
    
    cchat_id = api_ltoa(chat_id);

    message = send_document(bot, cchat_id, filename, caption,
                                 reply_to_message_id, reply_markup);

    ffree(cchat_id);

    return message;
}


/**
 * sendVideo
 * https://core.telegram.org/bots/api#sendvideo
 */
Message * send_video(Bot * bot, char * chat_id, char * filename, int32_t duration,
            int32_t width, int32_t height, char * caption, bool supports_streaming,
            int64_t reply_to_message_id, char * reply_markup){
    Message * message;

    IFile ifile;

    ifile.type = SENDVIDEO;

    /* Unique identifier for the target */
    ifile.video.chat_id = chat_id;

    /* Audio file to send */
    ifile.video.filename = filename;

    /* Duration of the audio in seconds */
    ifile.video.duration = IFILE_INT(duration);

    /* Video width */
    ifile.video.width = IFILE_INT(width);

    /* Video height */
    ifile.video.height = IFILE_INT(height);

    /* Audio caption, 0-200 characters */
    ifile.video.caption = caption;

    /* parse mode MODE_HTML or MODE_MARKDOWN */
    ifile.video.parse_mode = parse_mode[get_parse_mode()];


    ifile.video.supports_streaming = CONVERT_URL_BOOLEAN(supports_streaming);

    /* Sends the message silently */
    ifile.video.disable_notification = CONVERT_URL_BOOLEAN(get_notification());

    /* If the message is a reply, ID of the original message */
    ifile.video.reply_to_message_id = IFILE_LONG(reply_to_message_id);

    ifile.video.reply_markup = CONVERT_URL_STRING(reply_markup);

    MemStore * input;
    refjson *s_json;

    input = call_method_upload(bot->token, ifile);
    ffree(ifile.video.duration);
    ffree(ifile.video.width);
    ffree(ifile.video.height);
    ffree(ifile.video.reply_to_message_id);
    if(!input)
        return NULL;

    s_json = start_json(input->content);
    mem_store_free(input);
    if(!s_json)
        return NULL;

    message = message_parse(s_json->content);

    close_json(s_json);

    return message;
}



Message * send_video_chat(Bot * bot, int64_t chat_id, char * filename, int32_t duration,
            int32_t width, int32_t height, char * caption, bool supports_streaming,
            int64_t reply_to_message_id, char * reply_markup){

    Message * message;
    char * cchat_id;
    
    cchat_id = api_ltoa(chat_id);

    message = send_video(bot, cchat_id, filename, duration, width, height, caption,
                supports_streaming, reply_to_message_id, reply_markup);

    ffree(cchat_id);

    return message;
}


/**
 * sendVoice
 * https://core.telegram.org/bots/api#sendvoice
 */
Message * send_voice(Bot *bot, char * chat_id, char * filename, char * caption,
            int32_t duration, int64_t reply_to_message_id,
            char * reply_markup){
    Message * message;

    IFile ifile;

    ifile.type = SENDVOICE;

    /* Unique identifier for the target */
    ifile.voice.chat_id = chat_id;

    /* Audio file to send */
    ifile.voice.filename = filename;

    /* Audio caption, 0-200 characters */
    ifile.voice.caption = caption;

    ifile.voice.parse_mode = parse_mode[get_parse_mode()];

    /* Duration of the audio in seconds */
    ifile.voice.duration = IFILE_INT(duration);

    /* Sends the message silently */
    ifile.voice.disable_notification = CONVERT_URL_BOOLEAN(get_notification());

    /* If the message is a reply, ID of the original message */
    ifile.voice.reply_to_message_id = IFILE_LONG(reply_to_message_id);

    ifile.voice.reply_markup = CONVERT_URL_STRING(reply_markup);

    MemStore * input;
    refjson *s_json;

    input = call_method_upload(bot->token, ifile);
    ffree(ifile.voice.duration);
    ffree(ifile.voice.reply_to_message_id);
    if(!input)
        return NULL;

    s_json = start_json(input->content);
    mem_store_free(input);
    if(!s_json)
        return NULL;

    message = message_parse(s_json->content);

    close_json(s_json);

    return message;
}



Message * send_voice_chat(Bot *bot, int64_t chat_id, char * filename, char * caption, int32_t duration,
            int64_t reply_to_message_id, char * reply_markup){

    Message * message;
    char * cchat_id;
    
    cchat_id = api_ltoa(chat_id);

    message = send_voice(bot, cchat_id, filename, caption, duration,
                                 reply_to_message_id, reply_markup);

    ffree(cchat_id);

    return message;
}


/**
 * sendVideoNote
 * https://core.telegram.org/bots/api#sendvideonote
 */
Message * send_video_note(Bot * bot, char * chat_id, char * filename, int32_t duration,
            int32_t length, int64_t reply_to_message_id, char * reply_markup){
    Message * message;

    IFile ifile;

    ifile.type = SENDVIDEONOTE;

    /* Unique identifier for the target */
    ifile.videonote.chat_id = chat_id;

    /* Document to send */
    ifile.videonote.filename = filename;

    /* Duration of the audio in seconds */
    ifile.videonote.duration = IFILE_INT(duration);

    /* Video width and height */
    ifile.videonote.length = IFILE_INT(length);

    /* Sends the message silently */
    ifile.videonote.disable_notification = CONVERT_URL_BOOLEAN(get_notification());

    /* If the message is a reply, ID of the original message */
    ifile.videonote.reply_to_message_id = IFILE_LONG(reply_to_message_id);

    ifile.videonote.reply_markup = CONVERT_URL_STRING(reply_markup);

    MemStore * input;
    refjson *s_json;

    input = call_method_upload(bot->token, ifile);
    ffree(ifile.videonote.duration);
    ffree(ifile.videonote.length);
    ffree(ifile.videonote.reply_to_message_id);
    if(!input)
        return NULL;

    s_json = start_json(input->content);
    mem_store_free(input);
    if(!s_json)
        return NULL;

    message = message_parse(s_json->content);

    close_json(s_json);

    return message;
}



Message * send_video_note_chat(Bot * bot, int64_t chat_id, char * filename, int32_t duration,
            int32_t length, int64_t reply_to_message_id, char * reply_markup){

    Message * message;
    char * cchat_id;
    
    cchat_id = api_ltoa(chat_id);

    message = send_video_note(bot, cchat_id, filename, duration, length,
                reply_to_message_id, reply_markup);

    ffree(cchat_id);

    return message;
}



Error * show_error(){
    Error * error = get_error();

    return error;
}


/**
 * forwardMessage
 * https://core.telegram.org/bots/api#forwardmessage
 */
Message * forward_message_from (Bot * bot, int64_t chat_id, char * from_chat_id,
            int64_t message_id){

    Message * message;
    char * cchat_id;

    cchat_id = api_ltoa(chat_id);

    message =  forward_message(bot, cchat_id, from_chat_id, message_id);

    ffree(cchat_id);

    return message;
}



Message * forward_message_from_chat (Bot * bot, char * chat_id, int64_t from_chat_id, 
            int64_t message_id){

    Message * message;
    char * cfrom_chat_id;

    cfrom_chat_id = api_ltoa(from_chat_id);

    message =  forward_message(bot, chat_id, cfrom_chat_id, message_id);

    ffree(cfrom_chat_id);

    return message;
}



Message * forward_message (Bot * bot, char * chat_id, char * from_chat_id, 
            int64_t message_id){
    Message * message;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_forwardMessage, chat_id, from_chat_id,
        CONVERT_URL_BOOLEAN(get_notification()), message_id);

    if(!s_json)
        return NULL;

    message = message_parse(s_json->content);

    close_json(s_json);

    return message;
}



Message * forward_message_chat (Bot * bot, int64_t chat_id, int64_t from_chat_id, 
            int64_t message_id){

    Message * message;
    char * cchat_id, *cfrom_chat_id;

    cchat_id = api_ltoa(chat_id);

    cfrom_chat_id = api_ltoa(from_chat_id);

    message =  forward_message(bot, cchat_id, cfrom_chat_id, message_id);

    ffree(cchat_id);
    ffree(cfrom_chat_id);

    return message;
}


/**
 * sendLocation
 * https://core.telegram.org/bots/api#sendlocation
 */
Message * send_location (Bot * bot, char * chat_id, float latitude,
            float longitude, int32_t live_period, int64_t reply_to_message_id,
            char * reply_markup){
    Message * message;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_sendLocation,
        chat_id, latitude, longitude, live_period,
        CONVERT_URL_BOOLEAN(get_notification()),
        reply_to_message_id, CONVERT_URL_STRING(reply_markup));

    if(!s_json)
        return NULL;

    message = message_parse(s_json->content);

    close_json(s_json);

    return message;
}



Message * send_location_chat (Bot * bot, int64_t chat_id, float latitude, float logitude,
            int32_t live_period, int64_t reply_to_message_id, char * reply_markup){

    Message * message;
    char * cchat_id;

    cchat_id = api_ltoa(chat_id);

    message = send_location(bot, cchat_id, latitude, logitude, live_period,
                reply_to_message_id, reply_markup);

    ffree(cchat_id);

    return message;
}


/**
 * sendContact
 * https://core.telegram.org/bots/api#sendcontact
 */
Message * send_contact(Bot * bot, char * chat_id, char * phone_number, char * first_name,
            char * last_name, int64_t reply_to_message_id, char * reply_markup){
    refjson *s_json;
    Message * message;

    s_json = generic_method_call(bot->token, API_sendContact,
        chat_id, phone_number, first_name, last_name,
        CONVERT_URL_BOOLEAN(get_notification()),
        reply_to_message_id, CONVERT_URL_STRING(reply_markup));

    if(!s_json)
        return NULL;

    message = message_parse(s_json->content);

    close_json(s_json);

    return message;
}



Message * send_contact_chat(Bot * bot, int64_t chat_id, char * phone_number, char * first_name,
            char * last_name, int64_t reply_to_message_id, char * reply_markup){
    Message * message;
    char * cchat_id;

    cchat_id = api_ltoa(chat_id);

    message = send_contact(bot, cchat_id, phone_number, first_name, last_name,
        reply_to_message_id, reply_markup);

    ffree(cchat_id);

    return message;
}


/**
 * sendChatAction
 * https://core.telegram.org/bots/api#sendchataction
 */
int send_chat_action(Bot * bot, char * chat_id, char * action){
    refjson *s_json;
    int result;

    s_json = generic_method_call(bot->token, API_sendChatAction,
            chat_id, action);

    if(!s_json)
        return -1;

    result = json_is_true(s_json->content);

    return result > 0  ? 1 : 0;
}

int send_chat_action_chat(Bot * bot, int64_t chat_id, char * action){
    int result;
    char * cchat_id;

    cchat_id = api_ltoa(chat_id);

    result = send_chat_action(bot, cchat_id, action);

    ffree(cchat_id);

    return result;
}


/**
 * sendVenue
 * https://core.telegram.org/bots/api#sendvenue
 */
Message * send_venue(Bot * bot, char * chat_id, float latitude, float longitude,
            char * title, char * address, char * foursquare_id,
            int64_t reply_to_message_id, char * reply_markup){
    refjson *s_json;
    Message * message;

    s_json = generic_method_call(bot->token, API_sendVenue,
        chat_id, latitude, longitude, title, address, foursquare_id,
        CONVERT_URL_BOOLEAN(get_notification()),
        reply_to_message_id, CONVERT_URL_STRING(reply_markup));

    if(!s_json)
        return NULL;

    message = message_parse(s_json->content);

    close_json(s_json);

    return message;
}



Message * send_venue_chat(Bot * bot, int64_t chat_id, float latitude, float longitude,
            char * title, char * address, char * foursquare_id,
            int64_t reply_to_message_id, char * reply_markup){
    Message * message;
    char * cchat_id;

    cchat_id = api_ltoa(chat_id);

    message = send_venue(bot, cchat_id, latitude, longitude, title,
        address, foursquare_id, reply_to_message_id,
        reply_markup);

    ffree(cchat_id);

    return message;
}


/**
 * editMessageLiveLocation
 * https://core.telegram.org/bots/api#editmessagelivelocation
 */
Message * edit_message_live_location(Bot * bot, char * chat_id, int64_t message_id,
            char * inline_message_id, float latitude, float longitude, char * reply_markup){
    Message * message;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_editMessageLiveLocation,
        chat_id, message_id, inline_message_id, latitude, longitude, CONVERT_URL_STRING(reply_markup));

    if(!s_json)
        return NULL;

    message = message_parse(s_json->content);

    close_json(s_json);

    return message;
}

Message * edit_message_live_location_chat(Bot * bot, int64_t chat_id, int64_t message_id,
            char * inline_message_id, float latitude, float longitude, char * reply_markup){
    Message * message;
    char * cchat_id;

    cchat_id = api_ltoa(chat_id);

    message = edit_message_live_location(bot, cchat_id, message_id, inline_message_id,
        latitude, longitude, reply_markup);

    ffree(cchat_id);

    return message;
}

/**
 * stopMessageLiveLocation
 * https://core.telegram.org/bots/api#stopmessagelivelocation
 */
Message * stop_message_live_location(Bot * bot, char * chat_id, int64_t message_id,
            char * inline_message_id, char * reply_markup){
    refjson *s_json;
    Message * message;

    s_json = generic_method_call(bot->token, API_stopMessageLiveLocation,
                chat_id, message_id, inline_message_id,
                CONVERT_URL_STRING(reply_markup));

    if(!s_json)
        return NULL;

    message = message_parse(s_json->content);

    close_json(s_json);

    return message;
}

Message * stop_message_live_location_chat(Bot * bot, int64_t chat_id, int64_t message_id,
            char * inline_message_id, char * reply_markup){
    Message * message;
    char * cchat_id;

    cchat_id = api_ltoa(chat_id);

    message = stop_message_live_location(bot, cchat_id, message_id,
        inline_message_id, reply_markup);

    ffree(cchat_id);

    return message;
}

/**
 * editMessageText
 * https://core.telegram.org/bots/api#editmessagetext
 */
Message *edit_message_text(Bot *bot, char *chat_id, int64_t message_id,
    char *inline_message_id, char *text,
    bool disable_web_page_preview, char *reply_markup){
    Message *message;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_editMessageText, chat_id, message_id,
        inline_message_id, text, parse_mode[get_parse_mode()],
        CONVERT_URL_BOOLEAN(disable_web_page_preview), CONVERT_URL_STRING(reply_markup));

    if(!s_json)
        return NULL;

    message = message_parse(s_json->content);

    close_json(s_json);

    return message;
}

Message *edit_message_text_chat(Bot *bot, int64_t chat_id, int64_t message_id,
    char *inline_message_id, char *text, bool disable_web_page_preview,
    char *reply_markup){

    Message *message;
    char *cchat_id;

    cchat_id = api_ltoa(chat_id);

    message = edit_message_text(bot, cchat_id, message_id,
        inline_message_id, text,
        disable_web_page_preview, reply_markup);

    ffree(cchat_id);

    return message;
}

/**
 * editMessageCaption
 * https://core.telegram.org/bots/api#editmessagecaption
 */
Message *edit_message_caption(Bot *bot, char *chat_id,
    int64_t message_id, char *inline_message_id, char *caption,
    char *reply_markup){
    Message *message;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_editMessageCaption, chat_id,
        message_id, inline_message_id, caption, parse_mode[get_parse_mode()], CONVERT_URL_STRING(reply_markup));

    if(!s_json)
        return NULL;

    message = message_parse(s_json->content);

    close_json(s_json);

    return message;
}

Message *edit_message_caption_chat(Bot *bot, int64_t chat_id,
    int64_t message_id, char *inline_message_id, char *caption,
    char *reply_markup){
    Message *message;
    char *cchat_id;

    cchat_id = api_ltoa(chat_id);

    message = edit_message_caption(bot, cchat_id, message_id,
        inline_message_id, caption, reply_markup);

    ffree(cchat_id);

    return message;
}


/**
 * editMessageReplyMarkup
 * https://core.telegram.org/bots/api#editmessagereplymarkup
 */
Message *edit_message_reply_markup(Bot *bot, char *chat_id, int64_t message_id,
        char *inline_message_id, char *reply_markup){
    Message *message;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_editMessageReplyMarkup, chat_id,
        message_id, inline_message_id, CONVERT_URL_STRING(reply_markup));

    if(!s_json)
        return NULL;

    message = message_parse(s_json->content);

    close_json(s_json);

    return message;
}

Message *edit_message_reply_markup_chat(Bot *bot, int64_t chat_id, int64_t message_id,
        char *inline_message_id, char *reply_markup){
    Message *message;
    char *cchat_id;

    cchat_id = api_ltoa(chat_id);

    message = edit_message_reply_markup(bot, cchat_id, message_id,
        inline_message_id, reply_markup);

    ffree(cchat_id);

    return message;
}

/**
 * deleteMessage
 * https://core.telegram.org/bots/api#deletemessage
 */
bool delete_message(Bot *bot, char *chat_id, int64_t message_id){
    bool result;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_deleteMessage,
        chat_id, message_id);

    if(!s_json)
        return -1;

    result = json_is_true(s_json->content);

    close_json(s_json);

    return result;
}

bool delete_message_chat(Bot *bot, int64_t chat_id, int64_t message_id){
    bool result;
    char *cchat_id;

    cchat_id = api_ltoa(chat_id);

    result = delete_message(bot, cchat_id, message_id);

    ffree(cchat_id);

    return result;
}

/**
 * setChatStickerSet
 * https://core.telegram.org/bots/api#setchatstickerset
 */
bool set_chat_sticker_set(Bot *bot, char *chat_id, int64_t sticker_set_name){
    bool result;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_setChatStickerSet,
        chat_id, sticker_set_name);

    if(!s_json)
        return -1;

    result = json_is_true(s_json->content);

    close_json(s_json);

    return result;
}

bool set_chat_sticker_set_chat(Bot *bot, int64_t chat_id, int64_t sticker_set_name){
    bool result;
    char *cchat_id;

    cchat_id = api_ltoa(chat_id);

    result = set_chat_sticker_set(bot, cchat_id, sticker_set_name);

    ffree(cchat_id);

    return result;
}

/**
 * deleteChatStickerSet
 * https://core.telegram.org/bots/api#deletechatstickerset
 */
bool delete_chat_sticker_set(Bot *bot, char *chat_id){
    bool result;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_deleteChatStickerSet, chat_id);

    if(!s_json)
        return -1;

    result = json_is_true(s_json->content);

    close_json(s_json);

    return result;
}

bool delete_chat_sticker_set_chat(Bot *bot, int64_t chat_id){
    bool result;
    char *cchat_id;

    cchat_id = api_ltoa(chat_id);

    result = delete_chat_sticker_set(bot, cchat_id);

    ffree(cchat_id);

    return result;
}

/*
 * https://core.telegram.org/bots/api#answerinlinequery
 */
bool answer_inline_query( Bot *bot, char *inline_query_id, char *results, int64_t cache_time, bool is_personal,
    char *next_offset, char *switch_pm_text, char *switch_pm_parameter) {
    bool result;
    refjson *s_json;

    s_json = generic_method_call(bot->token, API_answerInlineQuery, inline_query_id, results,
        cache_time, is_personal, CONVERT_URL_STRING(next_offset), CONVERT_URL_STRING(switch_pm_text), CONVERT_URL_STRING(switch_pm_parameter));

    if(!s_json)
        return -1;

    result = json_is_true(s_json->content);

    close_json( s_json );

    return result;
}

void set_notification(bool disable_notification){
    notification = disable_notification > 0 ? 1 : 0;
}

bool get_notification(){
    return notification;
}

void set_parse_mode(int32_t id_mode){
    if(id_mode == 1){
        mode = 1;
    }
    else if(id_mode == 2){
        mode = 2;
    }
    else{
        mode = 0;
    }
}

int32_t get_parse_mode(){
    return mode;
}
