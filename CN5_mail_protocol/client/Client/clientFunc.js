function checkForMessages() {
    var numOfMsgs = 0
    numOfMsgs = background.checkIncMessages()
    console.log("number of msgs: " + numOfMsgs)
    if(numOfMsgs !== -1)            // if we received from c++ background error with sock or connection was properly closed
    {
       listModel.clear()
       for(var i=0; i<numOfMsgs; i++)
       {
          var msg = background.getMessage()
          if(msg === "") {
              console.log("js successfully recognised the mistake with receiving message!")
              break;
          }

          listModel.append({"msg": msg})
       }
    } else
    {
        console.log("we are disconnected from server!")
        listModel.clear()
        signIn.enabled = true
        incomingMsg.enabled = false
        deleteMsg.enabled = false
        signOut.enabled = false
    }
}
