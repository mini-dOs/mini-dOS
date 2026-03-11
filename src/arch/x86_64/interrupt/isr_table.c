#include <stdint.h>

/* CPU exceptions: vectors 0–31 */
extern void isr_stub_0();  extern void isr_stub_1();  extern void isr_stub_2();  extern void isr_stub_3();
extern void isr_stub_4();  extern void isr_stub_5();  extern void isr_stub_6();  extern void isr_stub_7();
extern void isr_stub_8();  extern void isr_stub_9();  extern void isr_stub_10(); extern void isr_stub_11();
extern void isr_stub_12(); extern void isr_stub_13(); extern void isr_stub_14(); extern void isr_stub_15();
extern void isr_stub_16(); extern void isr_stub_17(); extern void isr_stub_18(); extern void isr_stub_19();
extern void isr_stub_20(); extern void isr_stub_21(); extern void isr_stub_22(); extern void isr_stub_23();
extern void isr_stub_24(); extern void isr_stub_25(); extern void isr_stub_26(); extern void isr_stub_27();
extern void isr_stub_28(); extern void isr_stub_29(); extern void isr_stub_30(); extern void isr_stub_31();

/* Hardware IRQ stubs: vectors 32–255 (no error code) */
extern void isr_stub_32();  extern void isr_stub_33();  extern void isr_stub_34();  extern void isr_stub_35();
extern void isr_stub_36();  extern void isr_stub_37();  extern void isr_stub_38();  extern void isr_stub_39();
extern void isr_stub_40();  extern void isr_stub_41();  extern void isr_stub_42();  extern void isr_stub_43();
extern void isr_stub_44();  extern void isr_stub_45();  extern void isr_stub_46();  extern void isr_stub_47();
extern void isr_stub_48();  extern void isr_stub_49();  extern void isr_stub_50();  extern void isr_stub_51();
extern void isr_stub_52();  extern void isr_stub_53();  extern void isr_stub_54();  extern void isr_stub_55();
extern void isr_stub_56();  extern void isr_stub_57();  extern void isr_stub_58();  extern void isr_stub_59();
extern void isr_stub_60();  extern void isr_stub_61();  extern void isr_stub_62();  extern void isr_stub_63();
extern void isr_stub_64();  extern void isr_stub_65();  extern void isr_stub_66();  extern void isr_stub_67();
extern void isr_stub_68();  extern void isr_stub_69();  extern void isr_stub_70();  extern void isr_stub_71();
extern void isr_stub_72();  extern void isr_stub_73();  extern void isr_stub_74();  extern void isr_stub_75();
extern void isr_stub_76();  extern void isr_stub_77();  extern void isr_stub_78();  extern void isr_stub_79();
extern void isr_stub_80();  extern void isr_stub_81();  extern void isr_stub_82();  extern void isr_stub_83();
extern void isr_stub_84();  extern void isr_stub_85();  extern void isr_stub_86();  extern void isr_stub_87();
extern void isr_stub_88();  extern void isr_stub_89();  extern void isr_stub_90();  extern void isr_stub_91();
extern void isr_stub_92();  extern void isr_stub_93();  extern void isr_stub_94();  extern void isr_stub_95();
extern void isr_stub_96();  extern void isr_stub_97();  extern void isr_stub_98();  extern void isr_stub_99();
extern void isr_stub_100(); extern void isr_stub_101(); extern void isr_stub_102(); extern void isr_stub_103();
extern void isr_stub_104(); extern void isr_stub_105(); extern void isr_stub_106(); extern void isr_stub_107();
extern void isr_stub_108(); extern void isr_stub_109(); extern void isr_stub_110(); extern void isr_stub_111();
extern void isr_stub_112(); extern void isr_stub_113(); extern void isr_stub_114(); extern void isr_stub_115();
extern void isr_stub_116(); extern void isr_stub_117(); extern void isr_stub_118(); extern void isr_stub_119();
extern void isr_stub_120(); extern void isr_stub_121(); extern void isr_stub_122(); extern void isr_stub_123();
extern void isr_stub_124(); extern void isr_stub_125(); extern void isr_stub_126(); extern void isr_stub_127();
extern void isr_stub_128(); extern void isr_stub_129(); extern void isr_stub_130(); extern void isr_stub_131();
extern void isr_stub_132(); extern void isr_stub_133(); extern void isr_stub_134(); extern void isr_stub_135();
extern void isr_stub_136(); extern void isr_stub_137(); extern void isr_stub_138(); extern void isr_stub_139();
extern void isr_stub_140(); extern void isr_stub_141(); extern void isr_stub_142(); extern void isr_stub_143();
extern void isr_stub_144(); extern void isr_stub_145(); extern void isr_stub_146(); extern void isr_stub_147();
extern void isr_stub_148(); extern void isr_stub_149(); extern void isr_stub_150(); extern void isr_stub_151();
extern void isr_stub_152(); extern void isr_stub_153(); extern void isr_stub_154(); extern void isr_stub_155();
extern void isr_stub_156(); extern void isr_stub_157(); extern void isr_stub_158(); extern void isr_stub_159();
extern void isr_stub_160(); extern void isr_stub_161(); extern void isr_stub_162(); extern void isr_stub_163();
extern void isr_stub_164(); extern void isr_stub_165(); extern void isr_stub_166(); extern void isr_stub_167();
extern void isr_stub_168(); extern void isr_stub_169(); extern void isr_stub_170(); extern void isr_stub_171();
extern void isr_stub_172(); extern void isr_stub_173(); extern void isr_stub_174(); extern void isr_stub_175();
extern void isr_stub_176(); extern void isr_stub_177(); extern void isr_stub_178(); extern void isr_stub_179();
extern void isr_stub_180(); extern void isr_stub_181(); extern void isr_stub_182(); extern void isr_stub_183();
extern void isr_stub_184(); extern void isr_stub_185(); extern void isr_stub_186(); extern void isr_stub_187();
extern void isr_stub_188(); extern void isr_stub_189(); extern void isr_stub_190(); extern void isr_stub_191();
extern void isr_stub_192(); extern void isr_stub_193(); extern void isr_stub_194(); extern void isr_stub_195();
extern void isr_stub_196(); extern void isr_stub_197(); extern void isr_stub_198(); extern void isr_stub_199();
extern void isr_stub_200(); extern void isr_stub_201(); extern void isr_stub_202(); extern void isr_stub_203();
extern void isr_stub_204(); extern void isr_stub_205(); extern void isr_stub_206(); extern void isr_stub_207();
extern void isr_stub_208(); extern void isr_stub_209(); extern void isr_stub_210(); extern void isr_stub_211();
extern void isr_stub_212(); extern void isr_stub_213(); extern void isr_stub_214(); extern void isr_stub_215();
extern void isr_stub_216(); extern void isr_stub_217(); extern void isr_stub_218(); extern void isr_stub_219();
extern void isr_stub_220(); extern void isr_stub_221(); extern void isr_stub_222(); extern void isr_stub_223();
extern void isr_stub_224(); extern void isr_stub_225(); extern void isr_stub_226(); extern void isr_stub_227();
extern void isr_stub_228(); extern void isr_stub_229(); extern void isr_stub_230(); extern void isr_stub_231();
extern void isr_stub_232(); extern void isr_stub_233(); extern void isr_stub_234(); extern void isr_stub_235();
extern void isr_stub_236(); extern void isr_stub_237(); extern void isr_stub_238(); extern void isr_stub_239();
extern void isr_stub_240(); extern void isr_stub_241(); extern void isr_stub_242(); extern void isr_stub_243();
extern void isr_stub_244(); extern void isr_stub_245(); extern void isr_stub_246(); extern void isr_stub_247();
extern void isr_stub_248(); extern void isr_stub_249(); extern void isr_stub_250(); extern void isr_stub_251();
extern void isr_stub_252(); extern void isr_stub_253(); extern void isr_stub_254(); extern void isr_stub_255();

void *isr_stub_table[256] = {
    /* 0x00–0x07 */ isr_stub_0,   isr_stub_1,   isr_stub_2,   isr_stub_3,   isr_stub_4,   isr_stub_5,   isr_stub_6,   isr_stub_7,
    /* 0x08–0x0F */ isr_stub_8,   isr_stub_9,   isr_stub_10,  isr_stub_11,  isr_stub_12,  isr_stub_13,  isr_stub_14,  isr_stub_15,
    /* 0x10–0x17 */ isr_stub_16,  isr_stub_17,  isr_stub_18,  isr_stub_19,  isr_stub_20,  isr_stub_21,  isr_stub_22,  isr_stub_23,
    /* 0x18–0x1F */ isr_stub_24,  isr_stub_25,  isr_stub_26,  isr_stub_27,  isr_stub_28,  isr_stub_29,  isr_stub_30,  isr_stub_31,
    /* 0x20–0x27 */ isr_stub_32,  isr_stub_33,  isr_stub_34,  isr_stub_35,  isr_stub_36,  isr_stub_37,  isr_stub_38,  isr_stub_39,
    /* 0x28–0x2F */ isr_stub_40,  isr_stub_41,  isr_stub_42,  isr_stub_43,  isr_stub_44,  isr_stub_45,  isr_stub_46,  isr_stub_47,
    /* 0x30–0x37 */ isr_stub_48,  isr_stub_49,  isr_stub_50,  isr_stub_51,  isr_stub_52,  isr_stub_53,  isr_stub_54,  isr_stub_55,
    /* 0x38–0x3F */ isr_stub_56,  isr_stub_57,  isr_stub_58,  isr_stub_59,  isr_stub_60,  isr_stub_61,  isr_stub_62,  isr_stub_63,
    /* 0x40–0x47 */ isr_stub_64,  isr_stub_65,  isr_stub_66,  isr_stub_67,  isr_stub_68,  isr_stub_69,  isr_stub_70,  isr_stub_71,
    /* 0x48–0x4F */ isr_stub_72,  isr_stub_73,  isr_stub_74,  isr_stub_75,  isr_stub_76,  isr_stub_77,  isr_stub_78,  isr_stub_79,
    /* 0x50–0x57 */ isr_stub_80,  isr_stub_81,  isr_stub_82,  isr_stub_83,  isr_stub_84,  isr_stub_85,  isr_stub_86,  isr_stub_87,
    /* 0x58–0x5F */ isr_stub_88,  isr_stub_89,  isr_stub_90,  isr_stub_91,  isr_stub_92,  isr_stub_93,  isr_stub_94,  isr_stub_95,
    /* 0x60–0x67 */ isr_stub_96,  isr_stub_97,  isr_stub_98,  isr_stub_99,  isr_stub_100, isr_stub_101, isr_stub_102, isr_stub_103,
    /* 0x68–0x6F */ isr_stub_104, isr_stub_105, isr_stub_106, isr_stub_107, isr_stub_108, isr_stub_109, isr_stub_110, isr_stub_111,
    /* 0x70–0x77 */ isr_stub_112, isr_stub_113, isr_stub_114, isr_stub_115, isr_stub_116, isr_stub_117, isr_stub_118, isr_stub_119,
    /* 0x78–0x7F */ isr_stub_120, isr_stub_121, isr_stub_122, isr_stub_123, isr_stub_124, isr_stub_125, isr_stub_126, isr_stub_127,
    /* 0x80–0x87 */ isr_stub_128, isr_stub_129, isr_stub_130, isr_stub_131, isr_stub_132, isr_stub_133, isr_stub_134, isr_stub_135,
    /* 0x88–0x8F */ isr_stub_136, isr_stub_137, isr_stub_138, isr_stub_139, isr_stub_140, isr_stub_141, isr_stub_142, isr_stub_143,
    /* 0x90–0x97 */ isr_stub_144, isr_stub_145, isr_stub_146, isr_stub_147, isr_stub_148, isr_stub_149, isr_stub_150, isr_stub_151,
    /* 0x98–0x9F */ isr_stub_152, isr_stub_153, isr_stub_154, isr_stub_155, isr_stub_156, isr_stub_157, isr_stub_158, isr_stub_159,
    /* 0xA0–0xA7 */ isr_stub_160, isr_stub_161, isr_stub_162, isr_stub_163, isr_stub_164, isr_stub_165, isr_stub_166, isr_stub_167,
    /* 0xA8–0xAF */ isr_stub_168, isr_stub_169, isr_stub_170, isr_stub_171, isr_stub_172, isr_stub_173, isr_stub_174, isr_stub_175,
    /* 0xB0–0xB7 */ isr_stub_176, isr_stub_177, isr_stub_178, isr_stub_179, isr_stub_180, isr_stub_181, isr_stub_182, isr_stub_183,
    /* 0xB8–0xBF */ isr_stub_184, isr_stub_185, isr_stub_186, isr_stub_187, isr_stub_188, isr_stub_189, isr_stub_190, isr_stub_191,
    /* 0xC0–0xC7 */ isr_stub_192, isr_stub_193, isr_stub_194, isr_stub_195, isr_stub_196, isr_stub_197, isr_stub_198, isr_stub_199,
    /* 0xC8–0xCF */ isr_stub_200, isr_stub_201, isr_stub_202, isr_stub_203, isr_stub_204, isr_stub_205, isr_stub_206, isr_stub_207,
    /* 0xD0–0xD7 */ isr_stub_208, isr_stub_209, isr_stub_210, isr_stub_211, isr_stub_212, isr_stub_213, isr_stub_214, isr_stub_215,
    /* 0xD8–0xDF */ isr_stub_216, isr_stub_217, isr_stub_218, isr_stub_219, isr_stub_220, isr_stub_221, isr_stub_222, isr_stub_223,
    /* 0xE0–0xE7 */ isr_stub_224, isr_stub_225, isr_stub_226, isr_stub_227, isr_stub_228, isr_stub_229, isr_stub_230, isr_stub_231,
    /* 0xE8–0xEF */ isr_stub_232, isr_stub_233, isr_stub_234, isr_stub_235, isr_stub_236, isr_stub_237, isr_stub_238, isr_stub_239,
    /* 0xF0–0xF7 */ isr_stub_240, isr_stub_241, isr_stub_242, isr_stub_243, isr_stub_244, isr_stub_245, isr_stub_246, isr_stub_247,
    /* 0xF8–0xFF */ isr_stub_248, isr_stub_249, isr_stub_250, isr_stub_251, isr_stub_252, isr_stub_253, isr_stub_254, isr_stub_255,
};
