local introStage = 0;
local nextLine = 0;
local introStartMs = 0;
local errorStartMs = 0;
local lastIncrementMs = 0;
local nextLineMs = 0;

local skipText = false;

local memoryMB = 16;
local memAmountTarget = 1024 * memoryMB;
local memIncrement = 64;
local memIncrementTimer = 25;

function init()
    if game.IsLoadingSave() or config.GetStringOr("SawIntro1", "no") == "yes"
    then
        introStartMs = game.MsTime();
        introStage = 1;
        audio.StartLoop("Fan");
        audio.PlayMusic("Lost");
    else
        introStartMs = game.MsTime();
        introStage = 0;
        audio.PlayMusic("Forest");
    end
end

function responder(key)
    if(introStage == 0 and (game.MsTime() - introStartMs) >= 1000)
    then
        introStartMs = game.MsTime();
        
        game.Log("" .. introStartMs);
        
        introStage = 1;
        audio.StartLoop("Fan");
        audio.FadeMusic(1500);
    elseif skipText and game.HasSave() and introStage > 1 and key == 27
    then
        game.ToGame();
    end
end

function drawIntroShared()
    if introStage == 0 and not skipText then
        screen.DrawTextBox(0, game.GetText("IntroMessage"), game.GetText("IntroMessage2"), false);
    elseif introStage <= 1 then
        
        if((game.MsTime() - introStartMs) >= 3000)
        then
            introStartMs = game.MsTime();
            lastIncrementMs = introStartMs;
            introStage = 2;
            memAmount = memIncrement;
            audio.PlaySample("Beep");
        end
    elseif introStage == 2 or introStage == 3 then
        
        if(memAmount == memAmountTarget and (game.MsTime() - lastIncrementMs) >= 3500)
        then
            screen.HighRes();
            
            if(skipText and game.HasSave())
            then
                screen.DrawLineTextCentered(39, "Press ESCAPE to Skip");
                screen.DrawFillLineProp(0, 39, screen.CHAR_BLINK3, 80);
            end
        else
            screen.LowRes();
            
            if(skipText and game.HasSave())
            then
                screen.DrawLineTextCentered(24, "Press ESCAPE to Skip");
                screen.DrawFillLineProp(0, 24, screen.CHAR_BLINK3, 80);
            end
        end
        
        
        local target_msg = memAmountTarget .. " KB OK";
        local msg = memAmount .. " KB OK";
        screen.DrawLineText(1 + (#target_msg - #msg), 1, msg);
        while((game.MsTime() - lastIncrementMs) >= memIncrementTimer and memAmount < memAmountTarget)
        do
            lastIncrementMs = lastIncrementMs + memIncrementTimer;
            memAmount = memAmount + memIncrement;
        end
        
        if(memAmount == memAmountTarget)
        then
            screen.DrawLineTextFillProp(1, 2, "_", screen.CHAR_BLINK2);
            if(introStage == 2)
            then
                audio.PlaySample("Beep");
                introStage = 3;
            end
        end
        
        if(memAmount == memAmountTarget and (game.MsTime() - lastIncrementMs) >= 4000)
        then
            introStartMs = game.MsTime();
            nextLineMs = introStartMs;
            nextLine = 0;
            introStage = 4;
        end
    end
end

function drawIntro1()
    if introStage < 4
    then
        skipText = false;
        drawIntroShared();
    else
        if introStage == 4 or introStage == 5
        then
            while(game.MsTime() >= nextLineMs and nextLine < (game.NumInitTexts - game.NumRecoveryInitTexts))
            do
                nextLineMs = nextLineMs + game.GetInitText(nextLine).timer;
                if(game.GetInitText(nextLine).beep)
                then
                    audio.PlaySample("Beep");
                end
                nextLine = nextLine + 1;
            end
            
            local start = 0;
            local offset = 1;
            local numErrors = 0;
            local errorMaxLine1 = 8;
            local errorMaxLine2 = 13;
            local errorMaxLine3 = 16;
            local errorDelayMs = 1000;
            
            if(introStage == 4)
            then
                if(nextLine < 38)
                then
                    start = 0;
                    offset = 2;
                    screen.DrawLineText(1, 1, memAmountTarget .. " KB OK");
                else
                    start = nextLine - 38;
                    offset = 1;
                end
            elseif(introStage == 5)
            then
                start = ((game.NumInitTexts - game.NumRecoveryInitTexts) - 38);
                offset = 1;
                
                local errorsMs1 = 40;
                local errorsMs2 = 20;
                local errorsMs3 = 5;
                
                local errorMsLine1 = errorsMs1 * errorMaxLine1;
                local errorMsLine2 = errorsMs2 * errorMaxLine2;
                local errorMsLine3 = errorsMs3 * errorMaxLine3;
                
                local time = game.MsTime() - introStartMs;
                
                if(time < errorDelayMs)
                then
                    numErrors = 0;
                else
                    time = time - errorDelayMs;
                    if(time <= errorMsLine1)
                    then
                        numErrors = time // errorsMs1;
                    elseif(time <= (errorMsLine1 + errorMsLine2))
                    then
                        time = time - errorMsLine1;
                        numErrors = (time // errorsMs2) + errorMaxLine1;
                        start = start + 1;
                    else
                        time = time - errorMsLine1 + errorMsLine2;
                        numErrors = (time // errorsMs3) + errorMaxLine1 + errorMaxLine2;
                        
                        start = start + 1 + ((numErrors - (errorMaxLine1 + errorMaxLine2)) // errorMaxLine3);
                    end
                end
            end
            
            if(start < (game.NumInitTexts - game.NumRecoveryInitTexts))
            then
                local n = nextLine - start;
                for i = 0, n - 1, 1
                do
                    screen.DrawLineText(1, i + offset, game.GetInitText(start + i).text);
                    if(i == (n - 1) and nextLine < (game.NumInitTexts - game.NumRecoveryInitTexts))
                    then
                        screen.DrawLineTextFillProp(1 + #game.GetInitText(start + i).text, i + offset, ".", screen.CHAR_BLINK1);
                    end
                end
            else
                introStartMs = game.MsTime();
                glitchStartMs = introStartMs;
                introStage = 6;
                screen.RandFillReroll();
            end
            
            if(introStage == 5 and numErrors > 0)
            then
                local offsetX = 29;
                local offsetY = offset + (nextLine - start);
                
                game.Log("1, offsetY = " .. offsetY);
                
                offsetY = offsetY - 1;
                
                game.Log("2, offsetY = " .. offsetY);
                
                local n1 = math.min(numErrors, errorMaxLine1);
                for  i = 0, n1 - 1 , 1
                do
                    screen.DrawLineText(offsetX, offsetY, "ERROR");
                    offsetX = offsetX + 6;
                end
                
                game.Log("3, offsetY = " .. offsetY);
                
                local drawn = n1;
                
                if(numErrors > errorMaxLine1)
                then
                    offsetX = 0;
                    offsetY = offsetY + 1;
                    
                    game.Log("4, offsetY = " .. offsetY);
                    
                    local n2 = math.min(numErrors - errorMaxLine1, errorMaxLine2);
                    for i = 0, n2 - 1, 1
                    do
                        screen.DrawLineText(offsetX, offsetY, "ERROR");
                        offsetX = offsetX + 6;
                    end
                    
                    drawn = drawn + n2;
                end
                
                game.Log("5, offsetY = " .. offsetY);
                
                if(numErrors > (errorMaxLine1 + errorMaxLine2))
                then
                    local n2 = errorMaxLine3;
                    while(n2 == errorMaxLine3)
                    do
                        offsetX = 0;
                        offsetY = offsetY + 1;
                    
                        game.Log("6, offsetY = " .. offsetY);
                        
                        n2 = math.min(numErrors - drawn, errorMaxLine3);
                        for i = 0, n2 - 1, 1
                        do
                            screen.DrawLineText(offsetX, offsetY, "ERROR");
                            offsetX = offsetX + 5;
                        end
                        drawn = drawn + n2;
                    end
                end
            end
            
            if(nextLine == (game.NumInitTexts - game.NumRecoveryInitTexts) and introStage == 4)
            then
                introStartMs = game.MsTime();
                errorStartMs = introStartMs;
                introStage = 5;
                audio.PlaySample("Error");
            end
        end
        if introStage == 6
        then
            screen.RandFillPrint();
            if((game.MsTime() - glitchStartMs) >= 100)
            then
                glitchStartMs = game.MsTime();
                screen.RandFillReroll();
            end
            
            if((game.MsTime() - errorStartMs) >= 6500)
            then
                introStartMs = game.MsTime();
                introStage = 7;
            end
        end
        if introStage == 7
        then
            screen.DrawClear(32, screen.CHAR_INVERT1);
            if((game.MsTime() - introStartMs) >= 1000)
            then
                config.SetString("SawIntro1", "yes");
                game.ClearSave();
                game.Close();
            end
        end
    end
end

function drawIntro2()
    if introStage < 4
    then
        skipText = true;
        drawIntroShared();
    elseif introStage ==  4
    then
        if(game.HasSave())
        then
            screen.DrawLineTextCentered(39, "Press ESCAPE to Skip");
            screen.DrawFillLineProp(0, 39, screen.CHAR_BLINK3, 80);
        end
        
        screen.DrawLineText(1, 1, memAmountTarget .. " KB OK");
        screen.DrawLineTextCentered(3, "LAST BOOT FAILED");
        screen.DrawLineTextCentered(4, "ENTERING RECOVERY MODE");
        screen.DrawFillLineProp(0, 3, screen.CHAR_INVERT2, 80);
        screen.DrawFillLineProp(42, 3, screen.CHAR_INVERT2 | screen.CHAR_BLINK2, 6);
        screen.DrawFillLineProp(0, 4, screen.CHAR_INVERT2, 80);
        screen.DrawFillLineProp(38, 4, screen.CHAR_INVERT2 | screen.CHAR_BLINK2, 13);
        if((game.MsTime() - introStartMs) >= 2000)
        then
            introStartMs = game.MsTime();
            nextLine = 0;
            nextLineMs = introStartMs;
            introStage = 5;
            lineCountRecovery = 0;
            audio.PlaySample("Beep");
        end
    elseif introStage == 5
    then
        if(game.HasSave())
        then
            screen.DrawLineTextCentered(39, "Press ESCAPE to Skip");
            screen.DrawFillLineProp(0, 39, screen.CHAR_BLINK3, 80);
        end
        
        while(game.MsTime() >= nextLineMs and nextLine < game.NumInitTexts)
        do
            if(game.GetInitText(nextLine).recovery)
            then
                nextLineMs = nextLineMs + game.GetInitText(nextLine).timer;
                if(game.GetInitText(nextLine).beep)
                then
                    audio.PlaySample("Beep");
                end
                lineCountRecovery = lineCountRecovery + 1;
            end
            nextLine = nextLine + 1;
        end
        
        local start = 0;
        local offset = 0;
        
        if(lineCountRecovery < 35)
        then
            start = 0;
            offset = 5;
            screen.DrawLineText(1, 1, memAmountTarget .. " KB OK");
            screen.DrawLineTextCentered(3, "LAST BOOT FAILED");
            screen.DrawLineTextCentered(4, "ENTERING RECOVERY MODE");
            screen.DrawFillLineProp(0, 3, screen.CHAR_INVERT2, 80);
            screen.DrawFillLineProp(0, 4, screen.CHAR_INVERT2, 80);
        elseif(lineCountRecovery < 37)
        then
            start = 0;
            local extra = 0;
            if lineCountRecovery < 36
            then
                extra = 1;
            end
            
            
            offset = 3 + extra;
            
            screen.DrawLineTextCentered(1 + extra, "LAST BOOT FAILED");
            screen.DrawLineTextCentered(2 + extra, "ENTERING RECOVERY MODE");
            screen.DrawFillLineProp(0, 1 + extra, screen.CHAR_INVERT2, 80);
            screen.DrawFillLineProp(0, 2 + extra, screen.CHAR_INVERT2, 80);
        elseif(lineCountRecovery < 38)
        then
            start = 0;
            offset = 2;
            screen.DrawLineTextCentered(1, "ENTERING RECOVERY MODE");
            screen.DrawFillLineProp(0, 1, screen.CHAR_INVERT2, 80);
        else
            start = lineCountRecovery - 38;
            offset = 1;
        end
        
        local n = nextLine - start;
        local j = 0;
        for i = 0, n - 1, 1
        do
            if(game.GetInitText(start + i).recovery)
            then
                screen.DrawLineText(1, j + offset, game.GetInitText(start + i).text);
                if(i == (n - 1))
                then
                    screen.DrawLineTextFillProp(1 + #game.GetInitText(start + i).text, j + offset, ".", screen.CHAR_BLINK1);
                end
                j = j + 1;
            end
        end
        
        if(nextLine == game.NumInitTexts and game.MsTime() >= nextLineMs)
        then
            game.ToGame();
        end
    end
end

function tick()
    if game.IsLoadingSave() or config.GetStringOr("SawIntro1", "no") == "yes"
    then
        drawIntro2();
    else
        drawIntro1();
    end
end