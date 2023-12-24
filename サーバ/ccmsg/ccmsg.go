package ccmsg

import (
    "strconv"
)

//Action番号をenumとして使用
const (
    A_RoomIn = iota
    A_PlayerNumGet
    A_StopSync
    A_MoveSync
    A_CollisionSync
    A_Disconnect
)

// 固有メッセージ
const (
    kakkomae  string = "{"
    kakkoato  string = "}"
    actionstr string = "\"action\":"
    namestr   string = "\"name\":"
    shotXstr  string = "\"shotX\":"
    shotYstr  string = "\"shotY\":"
    posXstr   string = "\"posX\":"
    posYstr   string = "\"posY\":"
    shot1Xstr string = "\"shot1X\":"
    shot1Ystr string = "\"shot1Y\":"
    pos1Xstr  string = "\"pos1X\":"
    pos1Ystr  string = "\"pos1Y\":"
    shot2Xstr string = "\"shot2X\":"
    shot2Ystr string = "\"shot2Y\":"
    pos2Xstr  string = "\"pos2X\":"
    pos2Ystr  string = "\"pos2Y\":"
    hp1str    string = "\"hp1\":"
    hp2str    string = "\"hp2\":"
    playerstr string = "\"player\":"
)

// 部屋に入るメッセージを取得
func GetInRoomMsg() string{
    var msgF string

    action := strconv.Itoa(A_RoomIn)

    msgF = kakkomae
    msgF = msgF + actionstr + action
    msgF = msgF + kakkoato

    //fmt.Println(msgF)

    return msgF
}

// 退室メッセージ取得
func GetleaveRoomMsg() string{
    var msgF string

    action := strconv.Itoa(A_Disconnect)

    msgF = kakkomae
    msgF = msgF + actionstr + action
    msgF = msgF + kakkoato

    return msgF
}

// プレイヤー番号メッセージ取得
func GetPlayerIdMsg(message map[string]interface{}, playerNum int) string{
    var msgF string

    action := strconv.FormatFloat(message["action"].(float64), 'f', 2, 64)
    playerF := strconv.Itoa(playerNum)

    msgF = kakkomae
    msgF = msgF + actionstr + action + ","
    msgF = msgF + playerstr + playerF
    msgF = msgF + kakkoato

    //fmt.Println(playerF + "=" + msgF)

    return msgF
}

// ダメージ同期
func DamageSync(message map[string]interface{}) string{
    var msgF string

    action := strconv.FormatFloat(message["action"].(float64), 'f', 2, 64)

    msgF = kakkomae
    msgF = msgF + actionstr + action + ","

    return msgF
}

// 障害物衝突時同期メッセージ取得
func GetCollisionSyncMsg(message map[string]interface{}) string{
    var msgF string

    action := strconv.FormatFloat(message["action"].(float64), 'f', 2, 64)
    shotX := strconv.FormatFloat(message["shotX"].(float64), 'f', 2, 64)
    shotY := strconv.FormatFloat(message["shotY"].(float64), 'f', 2, 64)
    posX := strconv.FormatFloat(message["posX"].(float64), 'f', 2, 64)
    posY := strconv.FormatFloat(message["posY"].(float64), 'f', 2, 64)
    pNum := strconv.FormatFloat(message["number"].(float64), 'f', 2, 64)

    msgF = kakkomae
    msgF = msgF + actionstr + action + ","
    msgF = msgF + shotXstr + shotX + "," + shotYstr + shotY + ","
    msgF = msgF + posXstr + posX + "," + posYstr + posY + ","
    msgF = msgF + playerstr + pNum
    msgF = msgF + kakkoato

    return msgF
}

// プレイヤー停止後メッセージ取得
func GetStopSyncMsg(message map[string]interface{}) string{
    var msgF string

    name := message["name"].(string)
    action := strconv.FormatFloat(message["action"].(float64), 'f', 2, 64)
    shot1X := strconv.FormatFloat(message["shot1X"].(float64), 'f', 2, 64)
    shot1Y := strconv.FormatFloat(message["shot1Y"].(float64), 'f', 2, 64)
    pos1X  := strconv.FormatFloat(message["pos1X"].(float64), 'f', 2, 64)
    pos1Y  := strconv.FormatFloat(message["pos1Y"].(float64), 'f', 2, 64)
    shot2X := strconv.FormatFloat(message["shot2X"].(float64), 'f', 2, 64)
    shot2Y := strconv.FormatFloat(message["shot2Y"].(float64), 'f', 2, 64)
    pos2X  := strconv.FormatFloat(message["pos2X"].(float64), 'f', 2, 64)
    pos2Y  := strconv.FormatFloat(message["pos2Y"].(float64), 'f', 2, 64)
    hp1    := strconv.FormatFloat(message["hp1"].(float64), 'f', 2, 64)
    hp2    := strconv.FormatFloat(message["hp2"].(float64), 'f', 2, 64)

    msgF = kakkomae
    msgF = msgF + actionstr + action + ","
    msgF = msgF + namestr + "\"" + name +  "\"," + shot1Xstr + shot1X + "," + shot1Ystr + shot1Y + ","
    msgF = msgF + pos1Xstr + pos1X + "," + pos1Ystr + pos1Y + ","
    msgF = msgF + shot2Xstr + shot2X + "," + shot2Ystr + shot2Y + ","
    msgF = msgF + pos2Xstr + pos2X + "," + pos2Ystr + pos2Y + ","
    msgF = msgF + hp1str + hp1 + "," + hp2str + hp2
    
    return msgF
}

// プレイヤー動作同期メッセージ取得
func GetMoveSyncMsg(message map[string]interface{}) string{
    var msgF string

    name := message["name"].(string)
    action := strconv.FormatFloat(message["action"].(float64), 'f', 2, 64)
    shot1X := strconv.FormatFloat(message["shot1X"].(float64), 'f', 2, 64)
    shot1Y := strconv.FormatFloat(message["shot1Y"].(float64), 'f', 2, 64)
    pos1X := strconv.FormatFloat(message["pos1X"].(float64), 'f', 2, 64)
    pos1Y := strconv.FormatFloat(message["pos1Y"].(float64), 'f', 2, 64)
    shot2X := strconv.FormatFloat(message["shot2X"].(float64), 'f', 2, 64)
    shot2Y := strconv.FormatFloat(message["shot2Y"].(float64), 'f', 2, 64)
    pos2X := strconv.FormatFloat(message["pos2X"].(float64), 'f', 2, 64)
    pos2Y := strconv.FormatFloat(message["pos2Y"].(float64), 'f', 2, 64)

    msgF = kakkomae
    msgF = msgF + actionstr + action + ","
    msgF = msgF + namestr + "\"" + name +  "\"," + shot1Xstr + shot1X + "," + shot1Ystr + shot1Y + ","
    msgF = msgF + pos1Xstr + pos1X + "," + pos1Ystr + pos1Y + ","
    msgF = msgF + shot2Xstr + shot2X + "," + shot2Ystr + shot2Y + ","
    msgF = msgF + pos2Xstr + pos2X + "," + pos2Ystr + pos2Y
    msgF = msgF + kakkoato
    
    return msgF
}