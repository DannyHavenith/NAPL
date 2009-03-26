#if !defined( PROCESSOR_VISITOR_H)
#define PROCESSOR_VISITOR_H

class block_producer;
class block_consumer;
class block_mutator;
class block_multi_consumer;
class binary_block_processor;

struct processor_visitor
{
	virtual void Visit( block_producer *p) = 0;
	virtual void Visit( block_consumer *p) = 0;
	virtual void Visit( block_mutator *p) = 0;
	virtual void Visit( block_multi_consumer *p) = 0;
	virtual void Visit( binary_block_processor *p) = 0;
};
#endif //PROCESSOR_VISITOR_H
