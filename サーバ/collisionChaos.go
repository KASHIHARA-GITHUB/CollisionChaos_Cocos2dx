package main

import (
    "log"
    "encoding/json"
    "strconv"
    "net/http"
    "github.com/gorilla/websocket"
    "main/cclog"
    "main/ccmsg"
)

// 部屋とクライアント変数
var roomClients = make(map[int]map[*websocket.Conn]bool)
var clientNum int = -1

//Action番号をenumとして使用
const (
    A_RoomIn = iota
    A_PlayerNumGet
    A_StopSync
    A_MoveSync
    A_CollisionSync
    A_Disconnect
)

// Websocket初期設定
var upgrader = websocket.Upgrader{
    ReadBufferSize:  1024,
    WriteBufferSize: 1024,
    CheckOrigin: func(r *http.Request) bool {
        return true
    },
}

// メイン関数
func main() {
    // ログファイル作成
    cclog.Create()

    // HTTPハンドラセット
    http.HandleFunc("/", handleConnection)
    http.ListenAndServe(":8080", nil)
}

// Websocketハンドラー
func handleConnection(w http.ResponseWriter, r *http.Request) {

    // 変数を宣言
    var roomID     int
    var playerID   int
    var actionType float64
    var msgF       string
    var inFlag     bool = false

    log.Println("クライアントが接続しました")

    // HTTP通信からWebsocket通信へ更新
    conn, err := upgrader.Upgrade(w, r, nil)
    if err != nil {
        log.Println(err)
        return
    }

    // IDを取得
    roomID, playerID = getID()

    // 部屋を生成
    createRoom(roomID, conn)

    // 退室後の処理
    defer disconnect(roomID, conn)

    // 部屋に入る
    errFlag := inRoom(roomID, playerID, conn)
    
    if (!errFlag){
        return
    }

    log.Println("ゲーム開始")
    log.Println("部屋ID : " + strconv.Itoa(roomID) + " クライアントID :  " + strconv.Itoa(clientNum))

    // 通信処理
    for {
        // 受信処理
        messageType, p, err := conn.ReadMessage()

        // 何らかの通信で落ちた場合の処理(アプリケーションを落とした場合等)
        if err != nil {
            if (!inFlag) {
                clientNum++
            }

            // 退室処理
            errFlag := leaveRoom(roomID)
            if (!errFlag) {
                log.Println("退室失敗")
                cclog.WriteID(roomID, playerID)
            }

            log.Println("退室しました")
            cclog.WriteID(roomID, playerID)
            return
        }

        // 受信したメッセージを分割
        var message map[string]interface{}
        err = json.Unmarshal([]byte(p), &message)
        if err != nil {
            log.Println("メッセージの解析エラー:", err)
            return
        }

        // アクションメッセージ取得
        actionType = message["action"].(float64)
        msgF = getActionMsg(actionType, message, playerID)

        // プレイヤー番号セット時にフラグをセット
        if (actionType == A_PlayerNumGet){
            inFlag = true
        }

        // ログ書き込み
        cclog.WriteID(roomID, playerID)
        cclog.WriteMsg(string(msgF))

        // アクション処理
        sendActionMsg(roomID, messageType, msgF, conn, actionType)
    }
}

// IDを取得
func getID() (int, int)  {

    // ID変数を宣言
    var roomID int
    var playerID int

    // 部屋IDとプレイヤーIDを計算
    clientNum++
    roomID = clientNum / 2
    playerID = clientNum % 2

    return roomID, playerID
}

// 部屋を生成
func createRoom(roomID int, conn *websocket.Conn) {
    // 部屋を生成
    if roomClients[roomID] == nil {
        roomClients[roomID] = make(map[*websocket.Conn]bool)
    }
    roomClients[roomID][conn] = true
    //fmt.Println(roomClients)
}

// 部屋に入る
func inRoom(roomID int, playerID int, conn *websocket.Conn) bool {

    // メッセージ変数を宣言
    var msgF string

    // クライアントからメッセージ待ち
    messageType, _, err := conn.ReadMessage()
    if err != nil {
        return false
    }

    // 部屋に入るクライアントが2人、PlayerIDが1であれば部屋に入る
    if clients := roomClients[roomID]; len(clients) >= 2 && playerID ==  1 {
        // 部屋に入るメッセージ作成
        msgF = ccmsg.GetInRoomMsg()

        // 部屋内のクライアントにメッセージを送信
        for client := range clients {
            //クライアントにメッセージ送信
            err := client.WriteMessage(messageType, []byte(msgF))
            if err != nil {
                log.Println(err)
                return false
            }
        }
    }

    return true
}

// 退室
func leaveRoom(roomID int) bool {

    // メッセージ変数宣言
    var msgF string

    // 退室メッセージを取得
    msgF = ccmsg.GetleaveRoomMsg()

    // 部屋内のクライアントにメッセージ送信
    clients := roomClients[roomID]
    for client := range clients {
        err := client.WriteMessage(1, []byte(msgF))
        if err != nil {
            log.Println(err)
            return false
        }
    }

    return true
}

// Websocket切断処理
func disconnect(roomID int, conn *websocket.Conn){

    // 部屋IDからクライアントを削除
    if roomClients[roomID] != nil {
        delete(roomClients[roomID], conn)
        if len(roomClients[roomID]) == 0 {
            delete(roomClients, roomID)
        }
    }

    conn.Close()
}

// アクションメッセージ取得
func getActionMsg(actionType float64, message map[string]interface{}, playerID int) string {

    var msgF string

    switch actionType {
        case A_PlayerNumGet:
            msgF = ccmsg.GetPlayerIdMsg(message, playerID)
        case A_StopSync:
            msgF = ccmsg.GetStopSyncMsg(message)
        case A_MoveSync:
            msgF = ccmsg.GetMoveSyncMsg(message)
        case A_CollisionSync:
            msgF = ccmsg.GetCollisionSyncMsg(message)
    }

    return msgF
}

// アクションメッセージ送信
func sendActionMsg(roomID int, messageType int, msgF string, conn *websocket.Conn, actionType float64) {

    // 部屋内のクライアント全員にメッセージ送信
    if clients := roomClients[roomID]; clients != nil {
        for client := range clients {

            switch actionType {
                // プレイヤー番号をクライアントにセット
                case A_PlayerNumGet:
                    err :=conn.WriteMessage(messageType, []byte(msgF))
                    if err != nil {
                        log.Println(err)
                    }
                // 停止後プレイヤー位置同期
                case A_StopSync:
                    err := client.WriteMessage(messageType, []byte(msgF))
                    if err != nil {
                        log.Println(err)
                    }
                // プレイヤー衝突時同期
                case A_CollisionSync:
                    err := client.WriteMessage(messageType, []byte(msgF))
                    if err != nil {
                        log.Println(err)
                    }
            }
        }
    }
}