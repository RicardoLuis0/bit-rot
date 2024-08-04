local introStage = 0;
local nextLine = 0;
local introStartMs = 0;
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
        introStage = 1;
        audio.FadeMusic(500);
        audio.StartLoop("Fan");
        audio.PlayMusic("Lost");
    else
        introStage = 0;
        audio.FadeMusic(500);
        audio.PlayMusic("Forest");
    end
end

function responder(key)
    if(introStage == 0)
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
    if introStage == 0 or introStage == 1 then
        if introStage == 0
        then
            if not skipText
            then
                screen.DrawTextBox(0, game.GetText("IntroMessage"), game.GetText("IntroMessage2"), false);
                return;
            else
                introStage = 1;
            end
        end
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
    elseif introStage == 4 or introStage == 5
    then
        game.ToGame(); -- TODO
    end
end

function drawIntro2()
    if introStage < 4
    then
        skipText = true;
        drawIntroShared();
    elseif introStage == 4
    then
        game.ToGame(); -- TODO
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