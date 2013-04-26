
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
            /* return (event1->data.stream_start.encoding ==                    event2->data.stream_start.encoding); */

        case YAML_DOCUMENT_START_EVENT:
            if ((event1->data.document_start.version_directive && !event2->data.document_start.version_directive)
                    || (!event1->data.document_start.version_directive && event2->data.document_start.version_directive)
                    || (event1->data.document_start.version_directive && event2->data.document_start.version_directive
                        && (event1->data.document_start.version_directive->major != event2->data.document_start.version_directive->major
                            || event1->data.document_start.version_directive->minor != event2->data.document_start.version_directive->minor)))
                return 0;
            if ((event1->data.document_start.tag_directives.end - event1->data.document_start.tag_directives.start) !=
                    (event2->data.document_start.tag_directives.end - event2->data.document_start.tag_directives.start))
                return 0;
            for (k = 0; k < (event1->data.document_start.tag_directives.end - event1->data.document_start.tag_directives.start); k ++) {
                if ((strcmp((char *)event1->data.document_start.tag_directives.start[k].handle,
                                (char *)event2->data.document_start.tag_directives.start[k].handle) != 0)
                        || (strcmp((char *)event1->data.document_start.tag_directives.start[k].prefix,
                            (char *)event2->data.document_start.tag_directives.start[k].prefix) != 0))
                    return 0;
            }
            /* if (event1->data.document_start.implicit != event2->data.document_start.implicit)
                return 0; */
            return 1;

        case YAML_DOCUMENT_END_EVENT:
            return 1;
            /* return (event1->data.document_end.implicit ==
                    event2->data.document_end.implicit); */

        case YAML_ALIAS_EVENT:
            return (strcmp((char *)event1->data.alias.anchor,
                        (char *)event2->data.alias.anchor) == 0);

        case YAML_SCALAR_EVENT:
            if ((event1->data.scalar.anchor && !event2->data.scalar.anchor)
                    || (!event1->data.scalar.anchor && event2->data.scalar.anchor)
                    || (event1->data.scalar.anchor && event2->data.scalar.anchor
                        && strcmp((char *)event1->data.scalar.anchor,
                            (char *)event2->data.scalar.anchor) != 0))
                return 0;
            if ((event1->data.scalar.tag && !event2->data.scalar.tag
                        && strcmp((char *)event1->data.scalar.tag, "!") != 0)
                    || (!event1->data.scalar.tag && event2->data.scalar.tag
                        && strcmp((char *)event2->data.scalar.tag, "!") != 0)
                    || (event1->data.scalar.tag && event2->data.scalar.tag
                        && strcmp((char *)event1->data.scalar.tag,
                            (char *)event2->data.scalar.tag) != 0))
                return 0;
            if ((event1->data.scalar.length != event2->data.scalar.length)
                    || memcmp(event1->data.scalar.value, event2->data.scalar.value,
                        event1->data.scalar.length) != 0)
                return 0;
            if ((event1->data.scalar.plain_implicit != event2->data.scalar.plain_implicit)
                    || (event2->data.scalar.quoted_implicit != event2->data.scalar.quoted_implicit)
                    /* || (event2->data.scalar.style != event2->data.scalar.style) */)
                return 0;
            return 1;

        case YAML_SEQUENCE_START_EVENT:
            if ((event1->data.sequence_start.anchor && !event2->data.sequence_start.anchor)
                    || (!event1->data.sequence_start.anchor && event2->data.sequence_start.anchor)
                    || (event1->data.sequence_start.anchor && event2->data.sequence_start.anchor
                        && strcmp((char *)event1->data.sequence_start.anchor,
                            (char *)event2->data.sequence_start.anchor) != 0))
                return 0;
            if ((event1->data.sequence_start.tag && !event2->data.sequence_start.tag)
                    || (!event1->data.sequence_start.tag && event2->data.sequence_start.tag)
                    || (event1->data.sequence_start.tag && event2->data.sequence_start.tag
                        && strcmp((char *)event1->data.sequence_start.tag,
                            (char *)event2->data.sequence_start.tag) != 0))
                return 0;
            if ((event1->data.sequence_start.implicit != event2->data.sequence_start.implicit)
                    /* || (event2->data.sequence_start.style != event2->data.sequence_start.style) */)
                return 0;
            return 1;

        case YAML_MAPPING_START_EVENT:
            if ((event1->data.mapping_start.anchor && !event2->data.mapping_start.anchor)
                    || (!event1->data.mapping_start.anchor && event2->data.mapping_start.anchor)
                    || (event1->data.mapping_start.anchor && event2->data.mapping_start.anchor
                        && strcmp((char *)event1->data.mapping_start.anchor,
                            (char *)event2->data.mapping_start.anchor) != 0))
                return 0;
            if ((event1->data.mapping_start.tag && !event2->data.mapping_start.tag)
                    || (!event1->data.mapping_start.tag && event2->data.mapping_start.tag)
                    || (event1->data.mapping_start.tag && event2->data.mapping_start.tag
                        && strcmp((char *)event1->data.mapping_start.tag,
                            (char *)event2->data.mapping_start.tag) != 0))
                return 0;
            if ((event1->data.mapping_start.implicit != event2->data.mapping_start.implicit)
                    /* || (event2->data.mapping_start.style != event2->data.mapping_start.style) */)
                return 0;
            return 1;

        default:
            return 1;
    }
}

int print_output(char *name, unsigned char *buffer, size_t size, int count)
{
    FILE *file;
    char data[BUFFER_SIZE];
    size_t data_size = 1;
    size_t total_size = 0;
    if (count >= 0) {
        printf("FAILED (at the event #%d)\nSOURCE:\n", count+1);
    }
    file = fopen(name, "rb");
    assert(file);
    while (data_size > 0) {
        data_size = fread(data, 1, BUFFER_SIZE, file);
        assert(!ferror(file));
        if (!data_size) break;
        assert(fwrite(data, 1, data_size, stdout) == data_size);
        total_size += data_size;
        if (feof(file)) break;
    }
    fclose(file);
    printf("#### (length: %d)\n", total_size);
    printf("OUTPUT:\n%s#### (length: %d)\n", buffer, size);
    return 0;
}

int
main(int argc, char *argv[])
{
    int number;
    int canonical = 0;
    int unicode = 0;

    number = 1;
    while (number < argc) {
        if (strcmp(argv[number], "-c") == 0) {
            canonical = 1;
        }
        else if (strcmp(argv[number], "-u") == 0) {
            unicode = 1;
        }
        else if (argv[number][0] == '-') {
            printf("Unknown option: '%s'\n", argv[number]);
            return 0;
        }
        if (argv[number][0] == '-') {
            if (number < argc-1) {
                memmove(argv+number, argv+number+1, (argc-number-1)*sizeof(char *));
            }
            argc --;
        }
        else {
            number ++;
        }
    }

    if (argc < 2) {
        printf("Usage: %s [-c] [-u] file1.yaml ...\n", argv[0]);
        return 0;
    }

    for (number = 1; number < argc; number ++)
    {
        FILE *file;
        yaml_parser_t parser;
        yaml_emitter_t emitter;
        yaml_event_t event;
        unsigned char buffer[BUFFER_SIZE];
        size_t written = 0;
        yaml_event_t events[MAX_EVENTS];
        size_t event_number = 0;
        int done = 0;
        int count = 0;
        int error = 0;
        int k;
        memset(buffer, 0, BUFFER_SIZE);
        memset(events, 0, MAX_EVENTS*sizeof(yaml_event_t));

        printf("[%d] Parsing, emitting, and parsing again '%s': ", number, argv[number]);
        fflush(stdout);

        file = fopen(argv[number], "rb");
        assert(file);

        assert(yaml_parser_initialize(&parser));
        yaml_parser_set_input_file(&parser, file);
        assert(yaml_emitter_initialize(&emitter));
        if (canonical) {
            yaml_emitter_set_canonical(&emitter, 1);
        }
        if (unicode) {
            yaml_emitter_set_unicode(&emitter, 1);
        }
        yaml_emitter_set_output_string(&emitter, buffer, BUFFER_SIZE, &written);

        while (!done)
        {
            if (!yaml_parser_parse(&parser, &event)) {
                error = 1;
                break;
            }

            done = (event.type == YAML_STREAM_END_EVENT);
            assert(event_number < MAX_EVENTS);
            assert(copy_event(&(events[event_number++]), &event));
            assert(yaml_emitter_emit(&emitter, &event) || 
                    (yaml_emitter_flush(&emitter) && print_output(argv[number], buffer, written, count)));
            count ++;
        }

        yaml_parser_delete(&parser);
        assert(!fclose(file));
        yaml_emitter_delete(&emitter);

        if (!error)
        {
            count = done = 0;
            assert(yaml_parser_initialize(&parser));
            yaml_parser_set_input_string(&parser, buffer, written);

            while (!done)
            {
                assert(yaml_parser_parse(&parser, &event) || print_output(argv[number], buffer, written, count));
                done = (event.type == YAML_STREAM_END_EVENT);
                assert(compare_events(events+count, &event) || print_output(argv[number], buffer, written, count));
                yaml_event_delete(&event);
                count ++;
            }
            yaml_parser_delete(&parser);
        }

        for (k = 0; k < event_number; k ++) {
            yaml_event_delete(events+k);
        }

        printf("PASSED (length: %d)\n", written);
        print_output(argv[number], buffer, written, -1);
    }

    return 0;
}