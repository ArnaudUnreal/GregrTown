"""
GregrTown regression tests, run inside Unreal Editor (Python).

Usage from the editor Python console (or RiderLink ue_execute_python):
    exec(open(r"D:/Unreal_Projects/GregrTown/Tests/gregr_regression.py").read())

The suite runs asset checks immediately, then starts PIE and drives the player over
several frames (tick callback, nothing blocks the editor). Results are logged with the
prefix GREGR_TEST and a final line "GREGR_TESTS DONE passed/total", and written to
Saved/Tests/regression_report.txt.
"""

import unreal


def _run_suite():
    # Imports are local: the exec() namespace is gone once the editor ticks the callbacks.
    import builtins
    import os
    import unreal

    REPORT = os.path.join(unreal.Paths.project_saved_dir(), "Tests", "regression_report.txt")
    results = []

    def record(name, ok, detail=""):
        results.append((name, ok, detail))
        unreal.log("GREGR_TEST %s %s %s" % ("PASS" if ok else "FAIL", name, detail))

    def check(name, fn):
        try:
            ok, detail = fn()
        except Exception as e:  # a crashing test is a failing test
            ok, detail = False, "exception: %s" % e
        record(name, ok, detail)

    # ------------------------------------------------------------------ asset checks
    def t_abp_compiles():
        abp = unreal.load_asset("/Game/GregrTown/Characters/Ty/ABP_Ty")
        unreal.BlueprintEditorLibrary.compile_blueprint(abp)
        status = abp.get_editor_property("status")
        return status == unreal.BlueprintStatus.BS_UP_TO_DATE, str(status)

    def t_blendspace():
        bs = unreal.load_asset("/Game/GregrTown/Characters/Ty/BS_Ty_Locomotion")
        samples = bs.get_editor_property("sample_data")
        names = [s.get_editor_property("animation").get_name() for s in samples]
        return len(samples) == 3, ",".join(names)

    def t_locomotion_speed_pin():
        # Walk/Run blend space X axis must be driven by GroundSpeed (regression: it was wired to Direction).
        PL = unreal.BlueprintGraphPinLibrary
        node = unreal.find_object(None, "/Game/GregrTown/Characters/Ty/ABP_Ty.ABP_Ty:AnimGraph.AnimGraphNode_StateMachine_0.Locomotion.AnimStateNode_2.Walk / Run.AnimGraphNode_BlendSpacePlayer_0")
        for p in unreal.BlueprintEditorLibrary.list_all_pins(node):
            if str(PL.get_pin_name(p)) == "X":
                srcs = [str(PL.get_pin_name(q)) for q in PL.list_connected_pins(p)]
                return srcs == ["GroundSpeed"], "X <- %s" % srcs
        return False, "no X pin"

    def t_fall_loop_static():
        a = unreal.load_asset("/Game/GregrTown/Characters/Ty/Anims/Ty_Fall_Loop")
        r0 = unreal.AnimationLibrary.get_bone_pose_for_time(a, "rightleg", 0.0, False).rotation
        r1 = unreal.AnimationLibrary.get_bone_pose_for_time(a, "rightleg", a.get_play_length() * 0.5, False).rotation
        drift = r0.angular_distance(r1)
        return a.get_play_length() >= 0.5 and drift < 1e-3, "len %.2f drift %.4f" % (a.get_play_length(), drift)

    def t_gc_material_usage():
        ar = unreal.AssetRegistryHelpers.get_asset_registry()
        gcs = ar.get_assets_by_class(unreal.TopLevelAssetPath("/Script/GeometryCollectionEngine", "GeometryCollection"))
        missing = set()
        for a in gcs[:40]:  # sample, loading all 240 is slow
            gc = unreal.load_asset(str(a.package_name))
            for m in gc.get_editor_property("materials"):
                b = m.get_base_material() if m else None
                if b and not b.get_path_name().startswith("/Engine/") and not b.get_editor_property("used_with_geometry_collections"):
                    missing.add(b.get_name())
        return not missing, "missing usage: %s" % sorted(missing)

    def t_gc_remove_on_break():
        # Broken pieces must disappear about 2 s after breaking.
        ar = unreal.AssetRegistryHelpers.get_asset_registry()
        gcs = [a for a in ar.get_assets_by_class(unreal.TopLevelAssetPath("/Script/GeometryCollectionEngine", "GeometryCollection"))
               if str(a.package_name).startswith("/Game/")]
        bad = []
        for a in gcs[::12]:
            data = unreal.GregrDestructionEditorLibrary.get_remove_on_break(unreal.load_asset(str(a.package_name)))
            if data.x < 1.0 or data.y > 2.5:
                bad.append(str(a.asset_name))
        return not bad, "checked %d, bad %s" % (len(gcs[::12]), bad)

    def t_feedback_assets():
        missing = []
        for name in ["Building", "Props", "Vegetation"]:
            da = unreal.load_asset("/Game/GregrTown/Destruction/DA_Destruction_" + name)
            for prop in ["break_sound", "hit_sound", "break_effect"]:
                if not da or not da.get_editor_property(prop):
                    missing.append("%s.%s" % (name, prop))
        hammer = unreal.get_default_object(unreal.load_asset("/Game/GregrTown/Weapons/BP_Hammer").generated_class())
        for prop in ["impact_sound", "swing_sound", "impact_effect"]:
            if not hammer.get_editor_property(prop):
                missing.append("Hammer." + prop)
        hud = unreal.get_default_object(unreal.load_asset("/Game/GregrTown/Core/BP_GregrHUD").generated_class())
        for prop in ["score_sound", "countdown_tick_sound", "challenge_end_sound", "score_effect"]:
            if not hud.get_editor_property(prop):
                missing.append("HUD." + prop)
        gm = unreal.get_default_object(unreal.load_asset("/Game/GregrTown/Core/BP_GregrGameMode").generated_class())
        if gm.get_editor_property("hud_class").get_name() != "BP_GregrHUD_C":
            missing.append("GameMode.hud_class")
        return not missing, "missing %s" % missing

    def t_render_features_off():
        # Hardware ray tracing (Lumen) hung the GPU on the 4060 Laptop: keep it and Substrate disabled.
        # Virtual shadow maps page-faulted the GPU at startup (Nanite shadow culling): regular shadow maps instead.
        on = [cv for cv in ["r.RayTracing", "r.Lumen.HardwareRayTracing", "r.Substrate", "r.Shadow.Virtual.Enable"]
              if unreal.SystemLibrary.get_console_variable_int_value(cv) != 0]
        return not on, "enabled %s" % on

    def t_input_keys():
        # The pause key mapping was lost while editing IMC_Default (adding Run): every action needs its keys.
        imc = unreal.load_asset("/Game/Input/IMC_Default")
        mapped = {}
        for m in imc.get_editor_property("default_key_mappings").get_editor_property("mappings"):
            if m.get_editor_property("action"):
                mapped.setdefault(m.get_editor_property("action").get_name(), set()).add(str(m.get_editor_property("key").get_editor_property("key_name")))
        expected = {"IA_Settings": {"P"}, "IA_Run": {"LeftShift"}, "IA_Jump": {"SpaceBar"}, "IA_Attack": {"LeftMouseButton"}, "IA_Move": {"Z", "Q", "S", "D"}}
        missing = ["%s:%s" % (a, sorted(keys - mapped.get(a, set()))) for a, keys in expected.items() if not keys <= mapped.get(a, set())]
        return not missing, "missing %s" % missing

    def t_french_culture():
        # The packaged game formatted scores the English way ("13,736"): force and stage the French culture.
        with open(os.path.join(unreal.Paths.project_config_dir(), "DefaultGame.ini"), encoding="utf-8") as f:
            text = f.read()
        need = ["[Internationalization]\nCulture=fr", "+CulturesToStage=fr", "InternationalizationPreset=EFIGS"]
        missing = [n for n in need if n not in text]
        return not missing, "missing %s" % missing

    def t_no_vsm_overflow_text():
        # The VSM overflow perf hint was drawn over the game HUD.
        value = unreal.SystemLibrary.get_console_variable_int_value("r.Shadow.Virtual.AllowScreenOverflowMessages")
        return value == 0, "r.Shadow.Virtual.AllowScreenOverflowMessages=%d" % value

    def t_nanite_material_usage():
        # Without "Used with Nanite" saved on the base material, Nanite GCs drew the default checker material.
        # The editor sets the flag in memory on its own: also require the material package not to be dirty.
        eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        dirty = {p.get_name() for p in unreal.EditorLoadingAndSavingUtils.get_dirty_content_packages()}
        bases = set()
        for a in eas.get_all_level_actors():
            for c in a.get_components_by_class(unreal.StaticMeshComponent):
                if c.static_mesh and c.static_mesh.get_editor_property("nanite_settings").enabled:
                    bases.update(c.get_material(i).get_base_material() for i in range(c.get_num_materials()) if c.get_material(i))
        bad = sorted({b.get_name() for b in bases if b.get_path_name().startswith("/Game/")
                      and (not b.get_editor_property("used_with_nanite") or b.get_outermost().get_name() in dirty)})
        return not bad, "%d base materials, missing or unsaved Nanite usage %s" % (len(bases), bad)

    def t_gc_nanite():
        # Hit objects are swapped for their Geometry Collection: non-Nanite GCs piled up during play
        # and overflowed the virtual shadow map marking queue.
        ar = unreal.AssetRegistryHelpers.get_asset_registry()
        gcs = [a for a in ar.get_assets_by_class(unreal.TopLevelAssetPath("/Script/GeometryCollectionEngine", "GeometryCollection"))
               if str(a.package_name).startswith("/Game/")]
        solid = (unreal.BlendMode.BLEND_OPAQUE, unreal.BlendMode.BLEND_MASKED)
        dirty = {p.get_name() for p in unreal.EditorLoadingAndSavingUtils.get_dirty_content_packages()}
        missing, no_usage = [], set()
        for a in gcs[::12]:  # sample, loading all of them is slow
            gc = unreal.load_asset(str(a.package_name))
            if all(m.get_blend_mode() in solid for m in gc.get_editor_property("materials") if m) and not gc.get_editor_property("enable_nanite"):
                missing.append(gc.get_name())
            for m in gc.get_editor_property("materials"):
                b = m.get_base_material() if m else None
                if b and gc.get_editor_property("enable_nanite") and b.get_path_name().startswith("/Game/") \
                        and (not b.get_editor_property("used_with_nanite") or b.get_outermost().get_name() in dirty):
                    no_usage.add(b.get_name())
        return not missing and not no_usage, "checked %d, without Nanite %s, materials missing Nanite usage %s" % (len(gcs[::12]), missing, sorted(no_usage))

    def t_run_input():
        # Run lives in C++ (AGregrCharacter): BP_Ty must only set the input action, no run logic in its graph.
        bp = unreal.load_asset("/Game/GregrTown/Characters/Ty/BP_Ty")
        action = unreal.get_default_object(bp.generated_class()).get_editor_property("run_action")
        # Read the saved asset: nodes deleted in the editor stay findable in memory (undo buffer).
        path = os.path.join(unreal.Paths.project_content_dir(), "GregrTown/Characters/Ty/BP_Ty.uasset")
        with open(path, "rb") as f:
            data = f.read()
        leftovers = [s for s in (b"K2Node_EnhancedInputAction", b"JumpZVelocity", b"MaxWalkSpeed") if s in data]
        return action and action.get_name() == "IA_Run" and not leftovers, "action %s, BP leftovers %s" % (action, leftovers)

    def t_level_meshes_nanite():
        # Many non-Nanite meshes overflowed the virtual shadow map marking queue: opaque/masked level meshes use Nanite.
        eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        meshes = {c.static_mesh for a in eas.get_all_level_actors() for c in a.get_components_by_class(unreal.StaticMeshComponent)
                  if c.static_mesh and c.static_mesh.get_path_name().startswith("/Game/")}
        solid = (unreal.BlendMode.BLEND_OPAQUE, unreal.BlendMode.BLEND_MASKED)
        missing = [m.get_name() for m in meshes
                   if all(s.material_interface.get_blend_mode() in solid for s in m.static_materials if s.material_interface)
                   and not m.get_editor_property("nanite_settings").enabled]
        # Complex-as-simple collision is built from the Nanite fallback: a simplified fallback changes
        # where blows land (fewer pieces broke) and what the player collides with.
        coarse = [m.get_name() for m in meshes
                  if m.get_editor_property("nanite_settings").enabled and m.get_editor_property("nanite_settings").fallback_relative_error > 0
                  and m.get_editor_property("body_setup").get_editor_property("collision_trace_flag") == unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE]
        return not missing and not coarse, "%d meshes, without Nanite %s, coarse collision %s" % (len(meshes), missing[:10], coarse[:10])

    for name, fn in [("asset.gc_remove_on_break", t_gc_remove_on_break), ("asset.feedback_assets", t_feedback_assets),
                     ("config.render_features_off", t_render_features_off), ("asset.run_input", t_run_input),
                     ("asset.level_meshes_nanite", t_level_meshes_nanite), ("asset.gc_nanite", t_gc_nanite),
                     ("asset.nanite_material_usage", t_nanite_material_usage),
                     ("config.no_vsm_overflow_text", t_no_vsm_overflow_text), ("asset.input_keys", t_input_keys),
                     ("config.french_culture", t_french_culture)]:
        check(name, fn)

    for name, fn in [("asset.abp_compiles", t_abp_compiles), ("asset.blendspace_samples", t_blendspace),
                     ("asset.locomotion_speed_pin", t_locomotion_speed_pin), ("asset.fall_loop_static", t_fall_loop_static),
                     ("asset.gc_material_usage", t_gc_material_usage)]:
        check(name, fn)

    # ------------------------------------------------------------------ PIE checks
    perf = unreal.find_object(None, "/Script/UnrealEd.Default__EditorPerformanceSettings")
    throttle_was = perf.get_editor_property("bThrottleCPUWhenNotForeground")
    perf.set_editor_property("bThrottleCPUWhenNotForeground", False)
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)

    # Player settings are touched by the tests: keep the user's values and restore them at the end.
    user_settings = unreal.GregrGameUserSettings.get_gregr_user_settings()
    saved_settings = (user_settings.get_challenge_duration(), user_settings.get_master_volume())
    user_settings.set_challenge_duration(600)  # long enough to never end during the suite
    user_settings.save_settings()

    les.editor_request_begin_play()

    ctx = {}
    saved_records = {}

    def local(pawn, v):
        d = v - pawn.get_actor_location()
        return unreal.Vector(d.dot(pawn.get_actor_forward_vector()), d.dot(pawn.get_actor_right_vector()), d.z)

    def wait(seconds):
        t = 0.0
        while t < seconds:
            t += (yield)

    def scenario():
        # --- wait for PIE world and pawn
        t = 0.0
        while True:
            gw = ues.get_game_world()
            pawn = unreal.GameplayStatics.get_player_pawn(gw, 0) if gw else None
            if pawn:
                break
            t += (yield)
            if t > 20:
                record("pie.start", False, "no pawn after 20s")
                return
        yield from wait(1.0)
        pc = unreal.GameplayStatics.get_player_controller(gw, 0)
        skm = pawn.get_editor_property("mesh")
        move = pawn.get_movement_component()
        combat = pawn.get_combat_component()
        record("pie.start", True, pawn.get_class().get_name())

        # --- start menu: the game opens on the settings, paused, countdown not started, overlay hidden
        start_hud = pc.get_gregr_hud()
        start_menu = start_hud.get_settings_widget() if start_hud else None
        start_state = unreal.GameplayStatics.get_game_state(gw)
        if not start_menu or not start_hud.is_settings_visible():
            record("menu.start_shown", False, "no settings window at start")
        else:
            overlay = start_hud.get_hud_widget()
            record("menu.start_shown",
                   start_menu.is_start_menu() and str(start_menu.get_resume_label()) == "Jouer" and start_menu.has_quit_button()
                   and unreal.GameplayStatics.is_game_paused(gw) and not start_state.is_challenge_running()
                   and pc.get_editor_property("show_mouse_cursor") and overlay.get_visibility() == unreal.SlateVisibility.COLLAPSED,
                   "label %s, quit %s, paused %s, running %s, overlay %s" % (start_menu.get_resume_label(), start_menu.has_quit_button(),
                   unreal.GameplayStatics.is_game_paused(gw), start_state.is_challenge_running(), overlay.get_visibility()))
            start_menu.click_resume()
            yield from wait(0.3)
            record("menu.play_starts", not start_hud.is_settings_visible() and not unreal.GameplayStatics.is_game_paused(gw)
                   and start_state.is_challenge_running() and overlay.get_visibility() != unreal.SlateVisibility.COLLAPSED
                   and abs(start_state.get_remaining_time() - user_settings.get_challenge_duration()) < 2,
                   "running %s, %.1fs left, overlay %s" % (start_state.is_challenge_running(), start_state.get_remaining_time(), overlay.get_visibility()))

        attack_action = unreal.load_asset("/Game/Input/Actions/IA_Attack")

        def press_attack():
            # Local player subsystems are not reachable directly from Python: find the live one each time
            # (the level reload of the restart test creates a new player).
            for subsystem in unreal.ObjectIterator(unreal.EnhancedInputLocalPlayerSubsystem):
                if not subsystem.get_path_name().startswith("/Script/"):
                    subsystem.inject_input_vector_for_action(attack_action, unreal.Vector(1, 0, 0), [], [])

        # --- PIE must start at the PlayerStart, not at the editor camera
        starts = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.PlayerStart)
        if starts:
            d = pawn.get_actor_location() - starts[0].get_actor_location()
            d.z = 0
            record("pie.spawn_at_player_start", d.length() < 50, "distance %.0f cm" % d.length())
        else:
            record("pie.spawn_at_player_start", False, "no PlayerStart")

        # --- input mode / controller
        record("game.player_controller", isinstance(pc, unreal.GregrPlayerController) and not pc.get_editor_property("show_mouse_cursor"),
               "%s cursor=%s" % (pc.get_class().get_name(), pc.get_editor_property("show_mouse_cursor")))

        # --- score scale: integers 10..100 growing with object size
        destructibles = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.GCDestructibleActor)
        scored = sorted((a.get_destructible_size(), a.get_break_score()) for a in destructibles)
        scores = [s for _, s in scored]
        monotonic = all(scores[i] <= scores[i + 1] for i in range(len(scores) - 1))
        record("score.scale_range", scores and min(scores) == 10 and max(scores) == 100 and all(isinstance(s, int) for s in scores),
               "n=%d min %s max %s" % (len(scores), min(scores) if scores else "-", max(scores) if scores else "-"))
        record("score.grows_with_size", monotonic, "median %d pts" % scores[len(scores) // 2] if scores else "")

        # --- HUD: score at 0, countdown running from the configured duration
        hud = pc.get_gregr_hud()
        hud_widget = hud.get_hud_widget() if hud else None
        game_state = unreal.GameplayStatics.get_game_state(gw)
        records = None
        if not hud_widget or not hud_widget.is_in_viewport():
            record("hud.visible", False, "no HUD widget")
        else:
            record("hud.visible", True, "")
            record("hud.score_starts_at_zero", str(hud_widget.get_score_display()) == "Score : 0", str(hud_widget.get_score_display()))
            # Score to beat: force a known record for this duration (restored at the end).
            records = [r for r in unreal.ObjectIterator(unreal.RecordsSubsystem) if not r.get_path_name().startswith("/Script/")]
            records = records[-1] if records else None
            if records:
                duration = game_state.get_challenge_duration()
                saved_records[duration] = records.get_best_score(duration)
                saved_records[90] = records.get_best_score(90)
                records.set_best_score(duration, 100)
                game_state.refresh_score_to_beat()
            t0 = game_state.get_remaining_time()
            txt0 = str(hud_widget.get_timer_display())
            yield from wait(2.2)
            t1 = game_state.get_remaining_time()
            txt1 = str(hud_widget.get_timer_display())
            record("challenge.countdown", 595 <= t0 <= 600 and 1.5 <= t0 - t1 <= 3.0 and txt0 != txt1,
                   "%.1f -> %.1f, HUD %s -> %s" % (t0, t1, txt0, txt1))
            record("hud.score_to_beat", bool(records) and str(hud_widget.get_score_to_beat_display()) == u"À battre : 100",
                   str(hud_widget.get_score_to_beat_display()))

        # --- weapon held at the hand socket
        weapon = combat.get_weapon()
        if weapon:
            wm = weapon.get_editor_property("mesh")
            gap = (skm.get_socket_location("RightHandMiddle1Socket") - wm.get_socket_location("HammerHandSocket")).length()
            record("weapon.grip_socket_aligned", gap < 0.5, "gap %.3f cm" % gap)
        else:
            record("weapon.grip_socket_aligned", False, "no weapon")

        # --- locomotion: walk then run, legs must swing, speed must be reached
        def locomotion(name, scale, min_speed, min_stride):
            fwd = pawn.get_actor_forward_vector()
            xs, speed, t = [], 0.0, 0.0
            while t < 2.0:
                pawn.add_movement_input(fwd, scale, False)
                dt = (yield)
                t += dt
                if t > 0.8:
                    speed = max(speed, move.velocity.length())
                    xs.append(local(pawn, skm.get_socket_location("RightFoot")).x)
            stride = max(xs) - min(xs) if xs else 0
            record("anim.%s" % name, speed >= min_speed and stride >= min_stride,
                   "speed %.0f (>=%d) foot stride %.1f cm (>=%d)" % (speed, min_speed, stride, min_stride))
            return stride

        yield from wait(0.3)
        walk_stride = yield from locomotion("walk", 0.35, 120, 15)
        yield from wait(1.0)
        run_stride = yield from locomotion("run", 1.0, 400, 25)
        yield from wait(1.0)

        # --- idle: feet must stay planted
        xs, t = [], 0.0
        while t < 1.0:
            t += (yield)
            xs.append(local(pawn, skm.get_socket_location("RightFoot")).x)
        record("anim.idle_feet_planted", max(xs) - min(xs) < 8, "foot motion %.1f cm" % (max(xs) - min(xs)))

        # --- jump: legs tucked in the air, no jitter during fall loop, back to ground
        pawn.jump()
        rows, t = [], 0.0
        while t < 3.0:
            t += (yield)
            if move.is_falling():
                rows.append((t, (skm.get_socket_location("RightFoot") - skm.get_socket_location("Hips")).z))
        airtime = (rows[-1][0] - rows[0][0]) if rows else 0
        late = [z for tt, z in rows if rows and tt > rows[0][0] + 0.45]
        tuck = max(late) if late else -999
        jitter = max(abs(late[i + 1] - late[i]) for i in range(len(late) - 1)) if len(late) > 1 else 999
        record("anim.jump_airborne", airtime > 0.5 and not move.is_falling(), "airtime %.2fs" % airtime)
        record("anim.jump_legs_tucked", tuck > -50, "foot-hips z %.1f (> -50)" % tuck)
        record("anim.fall_loop_no_jitter", jitter < 1.0, "max step %.2f cm" % jitter)
        yield from wait(0.5)

        # --- run: faster, higher jumps, dolly zoom (wider FOV, shorter boom, same frame width at the character)
        import math
        cam = pawn.get_editor_property("follow_camera")
        boom = pawn.get_editor_property("camera_boom")
        frame = lambda: boom.get_editor_property("target_arm_length") * math.tan(math.radians(cam.field_of_view / 2))
        fov0, frame0 = cam.field_of_view, frame()
        # Put the pawn back afterwards: the attack test approaches its wall from where the pawn stands.
        start_location, start_rotation = pawn.get_actor_location(), pawn.get_actor_rotation()
        pawn.set_running(True)
        jump_run = move.get_editor_property("jump_z_velocity")
        speed, t = 0.0, 0.0
        fwd = pawn.get_actor_forward_vector()
        while t < 2.0:
            pawn.add_movement_input(fwd, 1.0, False)
            t += (yield)
            speed = max(speed, move.velocity.length())
        fov_run, frame_run = cam.field_of_view, frame()
        # Attacking while running slows down, the run speed must come back after the attack.
        press_attack()
        yield from wait(0.2)
        attack_speed = move.get_editor_property("max_walk_speed")
        yield from wait(2.0)
        after_attack = move.get_editor_property("max_walk_speed")
        pawn.set_running(False)
        yield from wait(1.5)
        pawn.teleport(start_location, start_rotation)
        yield from wait(0.5)
        record("move.run_speed", speed >= 950 and abs(after_attack - 1000) < 1 and attack_speed < 1000,
               "speed %.0f, max walk speed attacking %.0f then %.0f" % (speed, attack_speed, after_attack))
        record("move.run_jump", abs(jump_run - 1200) < 1 and abs(move.get_editor_property("jump_z_velocity") - 600) < 1
               and abs(move.get_editor_property("max_walk_speed") - 500) < 1, "jump z run %.0f" % jump_run)
        record("camera.run_dolly_zoom", fov_run > fov0 + 10 and abs(frame_run - frame0) < 1 and abs(cam.field_of_view - fov0) < 1,
               "fov %.1f -> %.1f -> %.1f, frame %.1f -> %.1f" % (fov0, fov_run, cam.field_of_view, frame0, frame_run))

        # --- fall recovery: put under the terrain, the player must be brought back to safe ground
        recovery = pawn.get_fall_recovery_component()
        land = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.LandscapeProxy)[0]
        here = pawn.get_actor_location()
        ground = unreal.SystemLibrary.line_trace_multi_for_objects(gw, here + unreal.Vector(0, 0, 20000), here - unreal.Vector(0, 0, 20000),
                                                                   [unreal.ObjectTypeQuery.ECC_WORLD_STATIC], True, [pawn], unreal.DrawDebugTrace.NONE, True)
        land_hits = [h.to_tuple()[4] for h in (ground or []) if h.to_tuple()[9] == land]
        if not land_hits:
            record("movement.recover_under_terrain", False, "no landscape under the player")
        else:
            safe = recovery.get_last_safe_location()
            count_before = recovery.get_recovery_count()
            pawn.teleport(unreal.Vector(here.x, here.y, land_hits[0].z - 300), pawn.get_actor_rotation())
            yield from wait(1.0)
            back = pawn.get_actor_location()
            record("movement.recover_under_terrain", recovery.get_recovery_count() == count_before + 1 and abs(back.z - safe.z) < 150,
                   "recoveries %d -> %d, back at %.0f (safe %.0f)" % (count_before, recovery.get_recovery_count(), back.z, safe.z))

        # --- stuck in a floor slab (packaged game: SM_floor_50, "is stuck and failed to move"): brought back
        stuck_spot = unreal.Vector(18924.06, 6614.39, 1523.87)
        count_before = recovery.get_recovery_count()
        pawn.set_actor_location(stuck_spot, False, True)
        yield from wait(0.05)
        inside_at_start = recovery.is_inside_geometry()
        t = 0.0
        while t < 2.0 and recovery.get_recovery_count() == count_before:
            t += (yield)
        yield from wait(0.3)
        record("movement.recover_stuck_in_floor",
               inside_at_start and recovery.get_recovery_count() == count_before + 1 and not recovery.is_inside_geometry(),
               "inside at start %s, recovered after %.1fs, inside now %s" % (inside_at_start, t, recovery.is_inside_geometry()))

        # --- attack: impact notify fires and breaks something in front of a wall
        target = [a for a in unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.GCDestructibleActor)
                  if a.get_actor_label() == "SM_wall_window_20"]
        if not target:
            record("combat.attack_breaks", False, "SM_wall_window_20 not found")
        else:
            o, _ = target[0].get_actor_bounds(False)
            loc = pawn.get_actor_location()
            # Approach from the PlayerStart side whatever the previous tests did with the pawn
            # (the run test moves it 20 m away and breaks things on its way).
            d = o - (starts[0].get_actor_location() if starts else loc)
            d.z = 0
            d = d.normal()
            spot = unreal.Vector(o.x - d.x * 190, o.y - d.y * 190, loc.z)
            pawn.set_actor_location(spot, False, True)
            pawn.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(spot, unreal.Vector(o.x, o.y, spot.z)), True)
            pc.set_control_rotation(pawn.get_actor_rotation())
            yield from wait(0.3)
            impacts = {"n": 0}
            def on_impact():
                impacts["n"] += 1
            combat.on_weapon_impact.add_callable(on_impact)
            # Real input path: inject the IA_Attack action as if the left mouse button was clicked.
            press_attack()
            yield from wait(0.2)
            started = combat.is_attacking()
            # Floating "+points" must pop right after the blow lands, then go away.
            t = 0.0
            while impacts["n"] == 0 and t < 2.0:
                t += (yield)
            yield from wait(0.1)
            floating_after_hit = hud_widget.get_active_floating_score_count() if hud_widget else -1
            target_visible_start = target[0].get_visible_piece_count()
            yield from wait(1.2)
            hit_actors = [a for a in unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.GCDestructibleActor) if a.get_hit_count() > 0]
            hits = [a.get_actor_label() for a in hit_actors]
            record("combat.attack_started", started, "")
            record("combat.impact_notify", impacts["n"] == 1, "impacts %d" % impacts["n"])
            record("combat.attack_breaks", len(hits) >= 1, "hit %s" % hits)
            record("combat.attack_finished", not combat.is_attacking(), "")
            # Debris does not block the player (it pushed the character into floor slabs).
            gcc = target[0].get_components_by_class(unreal.GeometryCollectionComponent)
            pawn_response = gcc[0].get_collision_response_to_channel(unreal.CollisionChannel.ECC_PAWN) if gcc else None
            record("destruct.debris_ignores_player", pawn_response == unreal.CollisionResponseType.ECR_IGNORE, "response to Pawn: %s" % pawn_response)
            expected = sum(a.get_break_score() for a in hit_actors)
            shown = str(hud_widget.get_score_display()) if hud_widget else ""
            shown_value = int("".join(c for c in shown if c.isdigit()) or -1)  # ignores thousand separators
            record("hud.score_after_attack", expected > 0 and shown_value == expected, "%s (expected %d)" % (shown, expected))
            record("hud.record_beaten_live", str(hud_widget.get_score_to_beat_display()) == "Record battu !",
                   "%s with score %d vs 100" % (str(hud_widget.get_score_to_beat_display()), shown_value))

            record("destruct.profile_rules",
                   target[0].get_effective_profile().get_name() == "DA_Destruction_Building", target[0].get_effective_profile().get_name())
            record("hud.floating_scores_pop", floating_after_hit >= 1, "%d on screen after the blow" % floating_after_hit)
            yield from wait(1.5)
            record("hud.floating_scores_vanish", hud_widget.get_active_floating_score_count() == 0,
                   "%d still on screen" % hud_widget.get_active_floating_score_count())
            # Pieces shrink away from ~2 s after breaking (remove-on-break timer 1..2.5 s): measuring at a fixed
            # time races the shrink, wait up to 4 s after the blow instead.
            since_blow = 2.8
            while target[0].get_visible_piece_count() >= target_visible_start * 0.7 and since_blow < 4.0:
                since_blow += (yield)
            visible_end = target[0].get_visible_piece_count()
            record("destruct.debris_removed", target_visible_start > 0 and visible_end < target_visible_start * 0.7,
                   "visible pieces %d -> %d, %.1f s after the blow" % (target_visible_start, visible_end, since_blow))

        # --- later hits: points only when they break more pieces
        def hud_score():
            return int("".join(c for c in str(hud_widget.get_score_display()) if c.isdigit()) or 0)

        def hit(actor, location, radius):
            imp = unreal.DestructionImpact()
            imp.location = location
            imp.direction = unreal.Vector(0, 0, -1)
            imp.radius = radius
            imp.strength = 1.0
            imp.instigator = pawn
            unreal.DestructionStatics.apply_destruction_impact_to_actor(actor, imp)

        here = pawn.get_actor_location()
        walls = [a for a in unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.GCDestructibleActor)
                 if a.get_actor_label().startswith("SM_wall_") and a.get_hit_count() == 0
                 and max(a.get_actor_bounds(False)[1].x, a.get_actor_bounds(False)[1].y) > 150]
        walls.sort(key=lambda a: (a.get_actor_location() - here).length())
        if not walls or not hud_widget:
            record("score.extra_break_points", False, "no wide wall")
        else:
            wall = walls[0]
            o, e = wall.get_actor_bounds(False)
            along = unreal.Vector(1, 0, 0) if e.x >= e.y else unreal.Vector(0, 1, 0)
            span = max(e.x, e.y) * 0.75
            left = unreal.Vector(o.x - along.x * span, o.y - along.y * span, o.z - e.z * 0.5)
            right = unreal.Vector(o.x + along.x * span, o.y + along.y * span, o.z - e.z * 0.5)
            hit(wall, left, 60.0)
            yield from wait(3.0)  # first break scored, its debris removed
            s0, b0 = hud_score(), wall.get_broken_piece_count()
            hit(wall, right, 60.0)
            yield from wait(0.8)
            s1, b1 = hud_score(), wall.get_broken_piece_count()
            record("score.extra_break_points", b1 > b0 and s1 - s0 == wall.get_extra_break_score(),
                   "%s: broke %d more, +%d (expected +%d)" % (wall.get_actor_label(), b1 - b0, s1 - s0, wall.get_extra_break_score()))
            yield from wait(3.0)  # debris of the second hit removed too
            s2, b2 = hud_score(), wall.get_broken_piece_count()
            hit(wall, right, 60.0)
            yield from wait(0.8)
            s3, b3 = hud_score(), wall.get_broken_piece_count()
            record("score.no_points_for_nothing", (b3 == b2) == (s3 == s2),
                   "hit in the emptied hole: broke %d, +%d" % (b3 - b2, s3 - s2))

        # --- loose parts fall, anchored base stays
        def column_probe(actor, z):
            o, e = actor.get_actor_bounds(False)
            types = [unreal.ObjectTypeQuery.ECC_DESTRUCTIBLE, unreal.ObjectTypeQuery.ECC_WORLD_STATIC,
                     unreal.ObjectTypeQuery.ECC_WORLD_DYNAMIC, unreal.ObjectTypeQuery.ECC_PHYSICS_BODY]
            for a, b in [((o.x - 150, o.y, z), (o.x + 150, o.y, z)), ((o.x, o.y - 150, z), (o.x, o.y + 150, z))]:
                hits = unreal.SystemLibrary.line_trace_multi_for_objects(gw, unreal.Vector(*a), unreal.Vector(*b), types, False, [pawn],
                                                                        unreal.DrawDebugTrace.NONE, True)
                if hits and any(h.to_tuple()[9] == actor for h in hits):
                    return True
            return False

        here = pawn.get_actor_location()
        columns = [a for a in unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.GCDestructibleActor)
                   if "column" in a.get_actor_label().lower() and a.get_hit_count() == 0 and a.get_actor_bounds(False)[1].z > 150]
        columns.sort(key=lambda a: (a.get_actor_location() - here).length())
        if len(columns) < 2:
            record("destruct.loose_parts_fall", False, "not enough columns")
        else:
            low_hit, untouched = columns[0], columns[1]
            o1, e1 = low_hit.get_actor_bounds(False)
            o2, e2 = untouched.get_actor_bounds(False)
            hit(low_hit, unreal.Vector(o1.x, o1.y, o1.z - e1.z + 60), 60.0)
            yield from wait(0.15)
            drop_start = low_hit.get_average_piece_drop()
            # Checked before the remove-on-break timer (1.5 s) so "fell" is not confused with "removed".
            yield from wait(1.1)
            drop_end = low_hit.get_average_piece_drop()
            record("destruct.loose_parts_fall", drop_end - drop_start > 40,
                   "%s (%.0f cm tall) hit at its base: pieces dropped %.0f cm on average" % (low_hit.get_actor_label(), 2 * e1.z, drop_end - drop_start))
            record("destruct.untouched_stays", untouched.get_hit_count() == 0 and column_probe(untouched, o2.z + e2.z - 40)
                   and column_probe(untouched, o2.z - e2.z + 30), "%s intact" % untouched.get_actor_label())

        # --- settings window: pauses the game, edits and saves the settings, resumes
        pc.open_settings()
        yield from wait(0.2)
        settings_widget = hud.get_settings_widget() if hud else None
        paused = unreal.GameplayStatics.is_game_paused(gw)
        record("settings.open_pauses", bool(settings_widget) and hud.is_settings_visible() and paused and pc.get_editor_property("show_mouse_cursor"),
               "visible %s paused %s" % (hud.is_settings_visible() if hud else None, paused))
        if settings_widget:
            settings_widget.set_duration_from_ui(95)   # snaps to 90
            settings_widget.set_volume_from_ui(0.5)
            record("settings.values", user_settings.get_challenge_duration() == 90 and abs(user_settings.get_master_volume() - 0.5) < 1e-3,
                   "duration %.0f volume %.2f" % (user_settings.get_challenge_duration(), user_settings.get_master_volume()))
            ini = os.path.join(unreal.Paths.project_saved_dir(), "Config", "WindowsEditor", "GameUserSettings.ini")
            saved_text = open(ini, encoding="utf-8", errors="ignore").read() if os.path.exists(ini) else ""
            record("settings.saved_to_ini", "ChallengeDuration=90" in saved_text and "MasterVolume=0.5" in saved_text, ini)
        pc.close_settings()
        yield from wait(0.2)
        record("settings.close_resumes", not hud.is_settings_visible() and not unreal.GameplayStatics.is_game_paused(gw)
               and not pc.get_editor_property("show_mouse_cursor"), "")

        # --- pause key: the IA_Settings action opens the pause menu and closes it again (while paused)
        settings_action = unreal.load_asset("/Game/Input/Actions/IA_Settings")

        def press_settings():
            for subsystem in unreal.ObjectIterator(unreal.EnhancedInputLocalPlayerSubsystem):
                if not subsystem.get_path_name().startswith("/Script/"):
                    subsystem.inject_input_vector_for_action(settings_action, unreal.Vector(1, 0, 0), [], [])

        press_settings()
        yield from wait(0.3)
        opened = hud.is_settings_visible() and unreal.GameplayStatics.is_game_paused(gw)
        press_settings()
        yield from wait(0.3)
        record("settings.pause_key_toggles", opened and not hud.is_settings_visible() and not unreal.GameplayStatics.is_game_paused(gw),
               "opened %s, then menu %s paused %s" % (opened, hud.is_settings_visible(), unreal.GameplayStatics.is_game_paused(gw)))

        # --- end of challenge: end screen with the score, player frozen
        score_before = str(hud_widget.get_score_display()) if hud_widget else ""
        game_state.set_remaining_time(0.5)
        yield from wait(1.0)
        end_widget = hud.get_end_screen_widget() if hud else None
        record("challenge.finished", game_state.is_challenge_finished() and not game_state.is_challenge_running(), "")
        record("challenge.end_screen", bool(end_widget) and hud.is_end_screen_visible() and str(end_widget.get_score_display()) == score_before,
               "%s vs HUD %s" % (str(end_widget.get_score_display()) if end_widget else None, score_before))
        record("menu.end_screen_has_quit", bool(end_widget) and end_widget.has_quit_button(), "")
        press_attack()
        yield from wait(0.3)
        record("challenge.player_frozen", not combat.is_attacking() and pc.get_editor_property("show_mouse_cursor"),
               "attacking after click: %s" % combat.is_attacking())
        final_value = int("".join(c for c in score_before if c.isdigit()) or 0)
        if records:
            best_now = records.get_best_score(game_state.get_challenge_duration())
            record("record.saved_on_finish", game_state.is_new_record() and best_now == final_value and end_widget.is_new_record_shown(),
                   "new record %s, best %d, final %d, shown %s" % (game_state.is_new_record(), best_now, final_value, end_widget.is_new_record_shown()))

        # --- restart: new level, score back to 0, countdown from the (new) duration
        pc.restart_challenge()
        t = 0.0
        new_pawn = None
        while t < 30:
            t += (yield)
            gw2 = ues.get_game_world()
            p2 = unreal.GameplayStatics.get_player_pawn(gw2, 0) if gw2 else None
            if p2 and p2 != pawn:
                new_pawn = p2
                break
        if not new_pawn:
            record("challenge.restart", False, "no new pawn after 30s")
        else:
            yield from wait(1.0)
            gw2 = ues.get_game_world()
            pc2 = unreal.GameplayStatics.get_player_controller(gw2, 0)
            hud2 = pc2.get_gregr_hud().get_hud_widget()
            gs2 = unreal.GameplayStatics.get_game_state(gw2)
            ok = str(hud2.get_score_display()) == "Score : 0" and 85 <= gs2.get_remaining_time() <= 90 and gs2.is_challenge_running()
            record("challenge.restart", ok, "%s, %.1fs left" % (str(hud2.get_score_display()), gs2.get_remaining_time()))
            # "Replay" goes straight into the game: no start menu.
            record("menu.replay_skips_start", not pc2.get_gregr_hud().is_settings_visible() and not unreal.GameplayStatics.is_game_paused(gw2),
                   "menu %s, paused %s" % (pc2.get_gregr_hud().is_settings_visible(), unreal.GameplayStatics.is_game_paused(gw2)))
            if records:
                expected_to_beat = u"À battre : %d" % records.get_best_score(90)
                record("record.per_duration", str(hud2.get_score_to_beat_display()) == expected_to_beat,
                       "%s (expected %s)" % (str(hud2.get_score_to_beat_display()), expected_to_beat))

        # Put the user's records back.
        if records:
            for duration, best in saved_records.items():
                records.set_best_score(duration, best if best > 0 else -1)

        # --- quit (last: it ends the PIE session) from the pause menu
        if new_pawn:
            pc2.open_settings()
            yield from wait(0.2)
            pause_menu = pc2.get_gregr_hud().get_settings_widget()
            pause_label = str(pause_menu.get_resume_label()) if pause_menu else ""
            if pause_menu:
                pause_menu.click_quit()
            t = 0.0
            while ues.get_game_world() and t < 5.0:
                t += (yield)
            record("menu.quit_ends_game", pause_label == "Continuer" and not ues.get_game_world(),
                   "pause menu label %s, game world after %.1fs: %s" % (pause_label, t, bool(ues.get_game_world())))

    gen = scenario()
    state = {"wait": 0.0, "started": False}

    def finish():
        unreal.unregister_slate_post_tick_callback(state["h"])
        les.editor_request_end_play()
        perf.set_editor_property("bThrottleCPUWhenNotForeground", throttle_was)
        user_settings.set_challenge_duration(saved_settings[0])
        user_settings.set_master_volume(saved_settings[1])
        user_settings.save_settings()
        passed = sum(1 for _, ok, _ in results if ok)
        os.makedirs(os.path.dirname(REPORT), exist_ok=True)
        with open(REPORT, "w", encoding="utf-8") as f:
            for name, ok, detail in results:
                f.write("%s %s %s\n" % ("PASS" if ok else "FAIL", name, detail))
            f.write("TOTAL %d/%d\n" % (passed, len(results)))
        unreal.log("GREGR_TESTS DONE %d/%d" % (passed, len(results)))

    def tick(dt):
        try:
            if not state["started"]:
                next(gen)
                state["started"] = True
            else:
                gen.send(dt)
        except StopIteration:
            finish()
        except Exception as e:
            record("suite.crash", False, "exception: %s" % e)
            finish()

    state["h"] = unreal.register_slate_post_tick_callback(tick)
    builtins._gregr_regression = (gen, state)  # keep alive


_run_suite()
