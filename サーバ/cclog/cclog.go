package cclog

import (
    "os"
    "log"
    "io"
    "strconv"
)

// ログファイル作成
func Create() {
    logFile := "collisionChaos.log"
    logfile, err := os.OpenFile(logFile, os.O_RDWR|os.O_CREATE|os.O_APPEND,0666)
    if err != nil {
        log.Fatalln(err)
    }
    multiLogFile := io.MultiWriter(os.Stdout, logfile)
    log.SetFlags(log.Ldate | log.Ltime | log.Lshortfile)
    log.SetOutput(multiLogFile)
}

// 部屋ID,プレイヤーIDをログに書き込む
func WriteID(roomID int, playerID int){
    log.Println("部屋ID : " + strconv.Itoa(roomID) + " プレイヤーID : "  + strconv.Itoa(playerID))
}

// メッセージ内容をログに書き込む
func WriteMsg(msg string){
    log.Println("メッセージ内容 : " + msg)
}