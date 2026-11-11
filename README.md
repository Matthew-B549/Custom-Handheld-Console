# Custom-Handheld-Console
This repo is a collection of the hardware and software which make-up my Custom Handheld Console. You can find my PCB files here. I have also documented my thoughts and improvements that could be made to the project.

I have created a helpful pixel editor tool for creating artwork to display on the console. This tool can be found here: https://github.com/Matthew-B549/PixelEditor



**My inspiration**:
This project was largely inspired by the Nintendo Game Boy.

**Agentic Coding**:
In this project, I experimented with Agentic coding. I found that the various Agents that I used were able to iterate incredibly quickly to varying levels of results. Individually, many of the games are decently coded and perform well. However, I found that the Agents struggled in creating a unified system and some of the code is messy at times.

**What Changed V1 to V2?**
The primary change from V1 to V2 was in size. I shrunk the PCB from 133mm by 170mm to 100mm by 100mm. (*Note: I have no Idea how I chose the dimensions 133 by 170 originally, but the board went from being comically large to a more standard size*) This dramatic decrease in size led to more intentional routing being necessary. I completely remapped the ESP32 pins from V1 to V2 to optimize and reduce vias and crossing traces. I also focused on signal integrity by placing resistors along high frequency SPI communication lines. Additionally, I routed the speaker signal trace intentionally to try and minimize noise and crosstalk.


**My Reflection**
This was my first project in which I made a PCB and I am exceptionally proud with the results. While there was much improvement from V1 to V2, there are still some issues I have come across when testing and using the Handheld Console. I have outlined some of the issues below:

* The ESP32 sometimes struggles to boot. This issue is likely due to weak battery connections and loosely attached power system on the back. On the back of the PCB, the battery, the buck boost converter, and the charging module are taped to the PCB. The PCB was not designed with this power system in mind. (In a similar fashion, the speaker is also taped to the back).

* I have not made a 3d printed case for this project. This is largely because of the irregular shape of the components mounted on the PCB. Large capacitors (among other components) poke through where the case would be. Again, the PCB was not designed specifically to work with a 3d printed case. These are all considerations that should be made earlier on in the process before ordering the PCB to ensure that both parts of the equation fit together.

* These design flaws heavily impact my experience when using the Console. They also make the Console very fragile and difficult to pass around and share.


I have included various pictures of the process and end result below.
